#include <retroshare/rsiface.h>   /* definition of iface */
#include <retroshare/rsinit.h>   /* definition of iface */
#include <iostream>
#include "chatserver.h"

int main(int argc, char **argv)
{
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

	NotifyBase *notify = new NotifyBase(); // discard all notifications!
	RsIface *iface = createRsIface(*notify);
	RsControl *rsServer = createRsControl(*iface, *notify);
	rsicontrol = rsServer ;

	std::string preferredId, gpgId, gpgName, gpgEmail, sslName;
	RsInit::getPreferedAccountId(preferredId);

	if (RsInit::getAccountDetails(preferredId, gpgId, gpgName, gpgEmail, sslName))
	{
		RsInit::SelectGPGAccount(gpgId);
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
	rsServer -> StartupRetroShare();

	// start chatserver
	Chatserver *chatserver = new Chatserver();
	while (true)
	{
			sleep(1);
			chatserver->tick();
	}
	return 0;
}

