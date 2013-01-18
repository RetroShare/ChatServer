#include "chatserver.h"

Chatserver::Chatserver(const unsigned int _checkForNewCertsInterval,
			   const unsigned int _maxFriends,
			   const unsigned int _ticksUntilLobbiesAreCreated)
: checkForNewCertsInterval(_checkForNewCertsInterval),
  maxFriends(_maxFriends),
  ticksUntilLobbiesAreCreated(_ticksUntilLobbiesAreCreated),
  tickCounter(0),
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
			friends.push_back(line);
	};
	ifs.close();
}

void Chatserver::saveChatServerStore(const std::string filename)
{
	std::ofstream ofs(filename);
	for (std::list<std::string>::iterator it = friends.begin(); it != friends.end(); ++it)
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
		std::string sslId, gpgId;
		try
		{
			bool success = rsPeers->loadCertificateFromString(cert, sslId, gpgId);
			if (!success)
			{
				std::cout << "ERROR: load certificate " << fileName << ", discarding it" << std::endl;
				removeFile(fileName);
				continue;
			}
		}
		catch (uint32_t error_code) // thrown in RsCertificate::RsCertificate(..)
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
		if (!rsPeers->addFriend(sslId, gpgId, RS_SERVICE_PERM_DISCOVERY))
		{
			std::cout << "ERROR: could not add friend." << std::endl;
			continue;
		}
		std::cout << "SUCCESS: added friend with gpg_id " << gpgId << std::endl;
		friends.push_back(gpgId);
		std::cout << "Chatserver: " << (removeFile(fileName) == false ? "couldn't " : "") << "remove " << fileName << std::endl;
		saveChatServerStore();
	}
}

void Chatserver::createOrRejoinLobbys()
{
	std::cout << "ChatServer::createOrRejoinLobbys()" << std::endl;

	std::vector<VisibleChatLobbyRecord> publicLobbies;
	rsMsgs->getListOfNearbyChatLobbies(publicLobbies);
	std::cout << "ChatServer: found " << publicLobbies.size() << " public lobbies." << std::endl;

	createOrRejoinLobby("Chatserver EN", "Welcome!", publicLobbies);
	createOrRejoinLobby("Chatserver DE", "Willkommen!", publicLobbies);
	createOrRejoinLobby("Chatserver ES", "Hola!", publicLobbies);
	createOrRejoinLobby("Chatserver FR", "Bonjour!", publicLobbies);
}

void Chatserver::createOrRejoinLobby(const std::string lobbyName, const std::string lobbyTopic, const std::vector<VisibleChatLobbyRecord> &publicLobbies)
{
	// range based for would be nicer, but don't want to use C++11 only for that
	bool ableToRejoin = false;
	for (std::vector<VisibleChatLobbyRecord>::const_iterator it = publicLobbies.begin(); it != publicLobbies.end(); ++it)
	{
		std::cout << "Chatserver is seeing the lobby " << it->lobby_name << ", " << it->lobby_topic << std::endl;
		if (it->lobby_name == lobbyName && it->lobby_topic == lobbyTopic)
		{
			// rejoin
			std::cout << "Chatserver: rejoined lobby " + lobbyName << std::endl;
			rsMsgs->joinVisibleChatLobby(it->lobby_id);
			ableToRejoin = true;
			return;
		}
	}

	if (!ableToRejoin)
	{
		// create new
		const std::list<std::string> emptyList = std::list<std::string>();
		std::cout << "Chatserver: creating new lobby " + lobbyName << std::endl;
		rsMsgs->createChatLobby(lobbyName, lobbyTopic, emptyList, RS_CHAT_LOBBY_PRIVACY_LEVEL_PUBLIC);
	}
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
