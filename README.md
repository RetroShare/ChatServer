retroshare-chatserver
=====================
This is a chat intro server for RetroShare, which (hopefully) deal with
the issues the current one from retroshare-nogui has.
How the chatserver works:
- adds all certificates in a given, hardcoded directory, after adding it 
  deletes them
- all added PGP ids are stored in a list, which is saved on disk 
  (chatserver_temporary_friends.txt)
- if a hardcoded number of friends is reached, the oldest gpgIDs are denied
  as friends
- after some startup time, the chatserver looks for visible chatlobbies and
  joins them, if he can still see e.g. the lobby "Chatserver DE"
- if the lobbys can't be joined, it creates new ones

How to setup:
- download the git repo to retroshare-code/trunk/retroshare-chatserver
  (must be that path, because we need libretroshare for include/linking)
- Chatserver changes in libretroshare
	/build/retroshare/libretroshare/src/pqip3peermgr.cc
	in p3peermgr.cc replace
	setServicePermissionFlags(gpg_id,service_flags)
	with 
	setServicePermissionFlags(gpg_id,RS_SERVICE_PERM_NONE) 
- change hardcoded constants in chatserver.h to what you want to, compile
  with qmake && make
- setup an account with retroshare-gui as usual, perhaps add some friends 
  which always will be allowed to connect (leeching lobbys and distributing 
  the chatserver lobbys even more far)
- copy config folder to server
- create the folder certificateFolder and the file chatserver_temporary_friends.txt
  (hardcoded in chatserver.h!)
- start with ./retroshare-chatserver -c configfolder
- setup w2c as webinterface, see https://github.com/cavebeat/rs-w2c
- make sure everything is correct with your permissions!

Paths:
- create Folders in ~/.retroshare
-  mkdir ~/.retroshare/chatserver This is where the files of chatserver are stored
-  mkdir ~/.retroshare/chatserver/NEWCERTS This is where www-data is storing entered pgp certificates
-  mkdir ~/.retroshare/chatserver/STORAGE This is where the serverkey, lobbys and a hyperlink are stored

Files: 
- the file serverkey.txt is stored in ~/.retroshare/chatserver/STORAGE and holds the RetroShare public certificate which is displayed in php frontend
- friendfifo.txt is stored in ~/.retroshare/chatserver/ and holds the FIFO Slots. chatserver reads them and deletes oldes entry from friendlist if there are more than 100 lines. 
- PHP Frontend https://github.com/cavebeat/rs-w2c/ stores all new certificates in ~/.retroshare/chatserver/NEWCERTS/
  chatserver reads all new pgp certificates from this directory and adds them to friendlist. And adds a line to ~/.retroshare/chatserver/friendfifo.txt
