#include "chatserver.h"

Chatserver::Chatserver(
		RsGxsId id,
		const unsigned int _checkForNewCertsInterval,
		const unsigned int _maxFriends,
		const unsigned int _ticksUntilLobbiesAreCreated)
: checkForNewCertsInterval(_checkForNewCertsInterval),
  maxFriends(_maxFriends),
  ticksUntilLobbiesAreCreated(_ticksUntilLobbiesAreCreated),
  tickCounter(0),
  ownId(id),
  friends()
{
	loadChatServerStore();
}

Chatserver::~Chatserver()
{
	// this isn't called at all !
}

void Chatserver::loadChatServerStore(const std::string filename)
{
	std::ifstream ifs(filename);
	std::string line;
	while (!ifs.eof())
	{
		getline(ifs, line);
		std::cout << "reading |" << line << "|" << ", length " << line.length() << std::endl;
		if (line.length() == 16)
			friends.push_back(RsPgpId(line));
	};
	ifs.close();
}

void Chatserver::saveChatServerStore(const std::string filename)
{
	std::ofstream ofs(filename);
	for (std::list<RsPgpId>::iterator it = friends.begin(); it != friends.end(); ++it)
	{
		std::cout << "writing |" << *it << "|" << std::endl;
		ofs << *it << std::endl;
	}
	ofs.close();
}

void Chatserver::tick()
{
	std::cout << "Chatserver::tick(" << ++tickCounter << "), currently " << numberOfFriends() << " friends." << std::endl;

	if (tickCounter % checkForNewCertsInterval == 0)
		checkForNewCertificates();

	//if (tickCounter == 5)
		//deployOwnCert();

	if (tickCounter == ticksUntilLobbiesAreCreated)
	{
		std::vector<VisibleChatLobbyRecord> dummy;
		// called it to trigger that all friends are asked
		rsMsgs->getListOfNearbyChatLobbies(dummy);
	}

	if (tickCounter == ticksUntilLobbiesAreCreated + 10)
		createOrRejoinLobbys();

	if (numberOfFriends() > maxFriends)
		removeOldestFriend();
}

void Chatserver::removeOldestFriend()
{
	std::cout << "ChatServer::removeOldestFriend()" << std::endl;

	while (numberOfFriends() > maxFriends)
	{
		if (!rsPeers->removeFriend(friends.front()))
		{
			std::cout << "FATAL: Could not remove friend with id " << friends.front() << std::endl;
		}
		friends.pop_front();
	};
	saveChatServerStore();
}

void Chatserver::checkForNewCertificates()
{
	std::vector<std::string> newCerts;
	if (getdir(certificatePath, newCerts) != 0)
	{
		std::cout << "ChatServer: reading directory failed" << std::endl;
		return;
	}
	if (newCerts.size() == 0)
	{
		std::cout << "Chatserver: no new certificates found." << std::endl;
		return;
	}

	for (std::vector<std::string>::const_iterator it = newCerts.begin(); it != newCerts.end(); ++it)
	{
		std::string fileName = certificatePath + (*it);
		std::cout << "adding certificate " << fileName << std::endl;
		// read file
		std::ifstream in(fileName);
		std::stringstream buffer;
		buffer << in.rdbuf();
		std::string cert(buffer.str());

		// we need to add "cert" now
		std::string errorString;
		RsPgpId gpgId;
		RsPeerId sslId;
		try
		{
			bool success = rsPeers->loadCertificateFromString(cert, sslId, gpgId, errorString);
			if (!success)
			{
				std::cout << "ERROR: load certificate " << fileName << ", error " << errorString << " discarding it" << std::endl;
				removeFile(fileName);
				continue;
			}
		}
		catch (uint32_t /*error_code*/) // thrown in RsCertificate::RsCertificate(..)
		{
			std::cout << "ERROR: caught exception while loading certificate " << fileName << ", discarding it" << std::endl;
			removeFile(fileName);
			continue;
		}

		if (rsPeers->isGPGAccepted(gpgId))
		{
			std::cout << "ERROR: peer with gpg id " << gpgId << " is already accepted. Discarding it." << std::endl;
			removeFile(fileName);
		}

		// TODO: enable discovery or not?
		if (!rsPeers->addFriend(sslId, gpgId, RS_NODE_PERM_NONE))
		{
			std::cout << "ERROR: could not add friend." << std::endl;
			continue;
		}
		std::cout << "SUCCESS: added friend with gpg_id " << gpgId << std::endl;
		// remove all existing gpgids from the friend fifo list before pushing a gpgid to the list
		// this is to prevent users from adding them too often to the chatserver and reducing the capacity
		// http://www.cplusplus.com/reference/list/list/remove/
		friends.remove(gpgId);
		// add the unique ID to the fifo list
		friends.push_back(gpgId);
		std::cout << "Chatserver: " << (removeFile(fileName) == false ? "couldn't " : "") << "remove " << fileName << std::endl;
		saveChatServerStore();
	}
}

void Chatserver::deployOwnCert()
{
	std::string key = rsPeers->GetRetroshareInvite(false);
	// std::cout << "%own% " << key << std::endl;

	const std::string filename = storagePath + "serverkey.txt";
	std::ofstream ofs;
	ofs.open(filename);
	ofs << key << std::endl;
	ofs.flush();
	ofs.close();
}

void Chatserver::createOrRejoinLobbys()
{
	std::cout << "ChatServer::createOrRejoinLobbys()" << std::endl;

	std::vector<VisibleChatLobbyRecord> publicLobbies;
	rsMsgs->getListOfNearbyChatLobbies(publicLobbies);
	std::cout << "ChatServer: found " << publicLobbies.size() << " public lobbies." << std::endl;

	// name, topic, id (optional), publicLobbies
	createOrRejoinLobby("Chatserver EN", "Welcome!", publicLobbies);
	createOrRejoinLobby("Chatserver DE", "Willkommen!", publicLobbies);
	createOrRejoinLobby("Chatserver ES", "Hola!", publicLobbies);
	createOrRejoinLobby("Chatserver FR", "Bonjour!", publicLobbies);
	/*
	createOrRejoinLobby("PirateParty", "Arrrrr, http://antiprism.eu/", publicLobbies);
	createOrRejoinLobby("RetroShare on IRC", "This Lobby is bridged to #retroshare on Freenode IRC. Connect with IRC WebChat http://webchat.freenode.net/?channels=retroshare", publicLobbies);
	*/
}

void inline Chatserver::createOrRejoinLobby(const std::string lobbyName, const std::string lobbyTopic, const std::vector<VisibleChatLobbyRecord> &publicLobbies)
{
	createOrRejoinLobby(lobbyName, lobbyTopic, "", publicLobbies);
}

void Chatserver::createOrRejoinLobby(const std::string lobbyName, const std::string lobbyTopic, const std::string lobbyId, const std::vector<VisibleChatLobbyRecord> &publicLobbies)
{
	// convert string to ChatLobbyId
	ChatLobbyId lid = 0;
	if(lobbyId != "")
		lid = strtoull(lobbyId.c_str(), NULL, 16);

	std::cout << "createOrRejoinLobby: " << std::endl;
	std::cout << "-- searching for name='" << lobbyName << "'";
	if(lobbyId != "")
		std::cout << " id=" << lobbyId;
	std::cout << std::endl;

	std::cout << "Chatserver is seeing:" << std::endl;
	// range based for would be nicer, but don't want to use C++11 only for that
	for (std::vector<VisibleChatLobbyRecord>::const_iterator it = publicLobbies.begin(); it != publicLobbies.end(); ++it)
	{
		std::cout << std::hex << "-- " << it->lobby_id << std::dec << ", " << it->lobby_name << ", " << it->lobby_topic << std::endl;
		if ((lobbyId != "" && it->lobby_id == lid) ||		// id based
			(lobbyId == "" && it->lobby_name == lobbyName))	// name based
		{
			// rejoin
			std::cout << "Chatserver: rejoined lobby " + lobbyName << std::endl;
			rsMsgs->joinVisibleChatLobby(it->lobby_id, ownId);
			return;
		}
	}

	// when we reach this part of the code we didn't find a lobby to join --> create new
	const std::set<RsPeerId> emptyList = std::set<RsPeerId>();
	std::cout << "Chatserver: creating new lobby " + lobbyName << std::endl;
	rsMsgs->createChatLobby(lobbyName, ownId, lobbyTopic, emptyList, RS_CHAT_LOBBY_FLAGS_PUBLIC);

}

int Chatserver::getdir (std::string dir, std::vector<std::string> &files)
{
	// copied from: http://www.linuxquestions.org/questions/programming-9/c-list-files-in-directory-379323/
	DIR *dp;
	struct dirent *dirp;
	if((dp = opendir(dir.c_str())) == NULL)
	{
		std::cout << "Error(" << errno << ") opening " << dir << std::endl;
		return errno;
	}

	while ((dirp = readdir(dp)) != NULL)
	{
		std::string fileName(dirp->d_name);
		if (fileName == ".") continue;
		if (fileName == "..") continue;
		if (fileName == ".svn") continue;
		files.push_back(std::string(dirp->d_name));
	};
	closedir(dp);
	return 0;
}

void Chatserver::removeAllFriends()
{
	while (! friends.empty())
	{
		if (!rsPeers->removeFriend(friends.front()))
		{
			std::cout << "ERROR: Could not remove friend with id " << friends.front() << std::endl;
			break;
		}
		friends.pop_front();
	}
	saveChatServerStore();
}

bool Chatserver::removeFile(const std::string &file)
{
	return (remove(file.c_str()) == 0);
}
