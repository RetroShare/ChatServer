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

// must end with /
const std::string certificatePath = "certs/";
const std::string temporaryFriendsFile = "chatserver_temporary_friends.txt";

class Chatserver
{
public:
	Chatserver(const unsigned int checkForNewCertsInterval = 10,
			   const unsigned int maxFriends = 20,
			   const unsigned int ticksUntilLobbiesAreCreated = 31);
	void tick();
	~Chatserver();
protected:
	const unsigned int checkForNewCertsInterval;
	const unsigned int maxFriends;
	const unsigned int ticksUntilLobbiesAreCreated;
	unsigned int tickCounter;

	void checkForNewCertificates();
	void removeOldestFriend();
	void removeAllFriends();

	// called after some startup time, see "ticksUntilLobbysAreCreated"
	void createOrRejoinLobbys();
	void createOrRejoinLobby(const std::string lobbyName, const std::string lobbyTopic, const std::vector<VisibleChatLobbyRecord> &publicLobbies);

	std::list<std::string> friends;
	size_t numberOfFriends() { return friends.size(); }

	void loadChatServerStore(const std::string filename = temporaryFriendsFile);
	void saveChatServerStore(const std::string filename = temporaryFriendsFile);

	// stupid helper function
	static int getdir(std::string dir, std::vector<std::string> &files);
	static bool removeFile(const std::string &file); // delete a file
};

#endif /* __CHATSERVER_H */
