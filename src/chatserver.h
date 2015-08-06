#ifndef __CHATSERVER_H
#define __CHATSERVER_H

// directory iteration, linux only!
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>

#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <list>
#include <vector>
#include <retroshare/rsmsgs.h>
#include <retroshare/rspeers.h>

/*
 * "certificatePath" needs to be an empty directory
 * The web interface puts all incoming certificates into this path.
 * this chatserver program reads them, adds them and deletes them afterwards, so check permissions!
 * the "temporaryFriendsFile" is only a txt file with gpg-ids in it.
 * Don't make temporaryFriendsFile accessible from outside, as it would reveal all current accepted gpg ids.
 * if there are more than "maxFriends" gpg ids, the oldest one is removed from the accepted list.
 */
const std::string certificatePath = "/home/user/.retroshare/chatserver/NEWCERTS/"; // must end with a slash !
const std::string storagePath = "/home/user/.retroshare/chatserver/STORAGE/"; // must end with a slash !
const std::string temporaryFriendsFile = "/home/user/.retroshare/chatserver/friend_fifo.txt";
const std::string name = "Chatserver";

class Chatserver
{
public:
	Chatserver(
			RsGxsId id,
			const unsigned int checkForNewCertsInterval = 30,
			const unsigned int maxFriends = 100,
			const unsigned int ticksUntilLobbiesAreCreated = 120);
	void tick();
	~Chatserver();
protected:
	const unsigned int checkForNewCertsInterval;
	/** max numbers of friends 
	this should be limited to 50 - 150 users, 
	so the load of the chatserver does not get too high because of too much connections. 
	and the users get kicked from time to time and are forced to create their own meshnet*/
	const unsigned int maxFriends; 		
	const unsigned int ticksUntilLobbiesAreCreated;
	unsigned int tickCounter;
	const RsGxsId ownId;

	void checkForNewCertificates();
	void removeOldestFriend();
	void removeAllFriends();

	void deployOwnCert();

	// called after some startup time, see "ticksUntilLobbysAreCreated"
	void createOrRejoinLobbys();
	void createOrRejoinLobby(const std::string lobbyName, const std::string lobbyTopic, const std::vector<VisibleChatLobbyRecord> &publicLobbies);
	void createOrRejoinLobby(const std::string lobbyName, const std::string lobbyTopic, const std::string lobbyId,  const std::vector<VisibleChatLobbyRecord> &publicLobbies);

	std::list<RsPgpId> friends;
	size_t numberOfFriends() { return friends.size(); }

	void loadChatServerStore(const std::string filename = temporaryFriendsFile);
	void saveChatServerStore(const std::string filename = temporaryFriendsFile);

	// stupid helper function
	static int getdir(std::string dir, std::vector<std::string> &files);
	static bool removeFile(const std::string &file); // delete a file
};

#endif /* __CHATSERVER_H */
