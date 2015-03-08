#include <retroshare/rsiface.h>     /* definition of iface */
#include <retroshare/rsinit.h>      /* definition of iface */
#include <retroshare/rsidentity.h>
#include <retroshare/rsconfig.h> 
#include <rsserver/rsaccounts.h>
#include <iostream>
#include "chatserver.h"
#include "MinimalNotify.h"
#include <dirent.h>
#include <ctime>
#include <unistd.h>

// copied from http://bytes.com/topic/c/answers/584434-check-directory-exists-c
bool DirectoryExists( const char* pzPath )
{
    if ( pzPath == NULL) return false;

    DIR *pDir;
    bool bExists = false;

    pDir = opendir (pzPath);

    if (pDir != NULL)
    {
        bExists = true;
        (void) closedir (pDir);
    }

    return bExists;
}

bool file_writable(const char * filename)
{
	if (FILE * file = fopen(filename, "rw"))
	{
		fclose(file);
		return true;
	}
	return false;
}

int generateGxsId(const std::string& name) {
	uint32_t token;
	RsIdentityParameters params;
	params.isPgpLinked = false;
	params.nickname = name;
	rsIdentity->createIdentity(token, params);

	// waiting for 10 seconds
	uint counter = 0;
	while (rsIdentity->getTokenService()->requestStatus(token) != RsTokenService::GXS_REQUEST_V2_STATUS_COMPLETE && counter++ < 10)
		usleep(1 * 1000 * 1000);

	if(rsIdentity->getTokenService()->requestStatus(token) != RsTokenService::GXS_REQUEST_V2_STATUS_COMPLETE)
	{
		std::cerr << "Error: can't generate GXS Id" << std::endl;
		return 1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	if (!DirectoryExists(certificatePath.c_str()))
	{
		std::cout << "hardcoded certificatePath " << certificatePath << " doesn't exist!" << std::endl;
		return 1;
	}
	if (!file_writable(storagePath.c_str()))
	{
		std::cout << "hardcoded storagePath " << storagePath << " doesn't exist or isn't writable!" << std::endl;
		return 1;
	}
	if (!file_writable(temporaryFriendsFile.c_str()))
	{
		std::cout << "hardcoded temporary friends file " << temporaryFriendsFile << " doesn't exist or isn't writable!" << std::endl;
		return 1;
	}

	// startup libretroshare
	RsInit::InitRsConfig();
	int initResult = RsInit::InitRetroShare(argc, argv, true);
	if (initResult < 0)
	{
		/* Error occured */
		switch (initResult)
		{
			case RS_INIT_AUTH_FAILED:
				std::cerr << "RsInit::InitRetroShare AuthGPG::InitAuth failed" << std::endl;
				break;
			default:
				/* Unexpected return code */
				std::cerr << "RsInit::InitRetroShare unexpected return code " << initResult << std::endl;
				break;
		};
		return 1;
	}


	RsControl::earlyInitNotificationSystem() ;
	NotifyTxt *notify = new NotifyTxt() ;
	rsNotify->registerNotifyClient(notify);


	RsPeerId preferredId;
	RsPgpId gpgId;
	std::string gpgName, gpgEmail, sslName;

	RsAccounts::GetPreferredAccountId(preferredId);

	if (RsAccounts::GetAccountDetails(preferredId, gpgId, gpgName, gpgEmail, sslName))
	{
		rsAccounts->SelectPGPAccount(gpgId);
	}

	/* Key + Certificate are loaded into libretroshare */

	std::string error_string ;
	int retVal = RsInit::LockAndLoadCertificates(false,error_string);
	switch(retVal)
	{
		case 0:	break;
		case 1:	std::cerr << "Error: another instance of retroshare is already using this profile" << std::endl;
				return 1;
		case 2: std::cerr << "An unexpected error occurred while locking the profile" << std::endl;
				return 1;
		case 3: std::cerr << "An error occurred while login with the profile" << std::endl;
				return 1;
		default: std::cerr << "Main: Unexpected switch value " << retVal << std::endl;
				return 1;
	}

	/* Start-up libretroshare server threads */
	RsControl::instance() -> StartupRetroShare();

	/* Disable all Turtle Routing and tunnel requests */
	rsConfig->setOperatingMode(RS_OPMODE_NOTURTLE);

	// get GXS Id
	RsGxsId id;
	std::list<RsGxsId> ids;
	rsIdentity->getOwnIds(ids);

	if(ids.empty()) {
		// generate a new ID
		if(generateGxsId(name) != 0)
			return 1;
		rsIdentity->getOwnIds(ids);
		id = ids.front();
	} else {
		// find gxs id with currect name
		std::list<RsGxsId>::iterator it;
		RsIdentityDetails details;
		for(it = ids.begin(); it != ids.end(); ++it) {
			rsIdentity->getIdDetails(*it, details);
			if(details.mNickname == name)
				id = *it;
		}

		// assume that the correct ID isn't generated yet
		if(id.isNull())
		{
			if(generateGxsId(name) != 0)
				return 1;
			rsIdentity->getOwnIds(ids);
			id = ids.front();
		}
	}

	if(id.isNull())
	{
		std::cerr << "Error: can't find GXS Id - available IDs are:" << std::endl;
		return 1;
	}
	rsMsgs->setDefaultIdentityForChatLobby(id);

	// start chatserver
	Chatserver *chatserver = new Chatserver(id);
	while (true)
	{
			sleep(1);
			chatserver->tick();
	}
	return 0;
}

