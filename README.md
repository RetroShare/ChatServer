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
- change hardcoded constants in chatserver.h to what you want to, compile
  with qmake && make
- setup an account with retroshare-gui as usual, perhaps add some friends 
  which always will be allowed to connect (leeching lobbys and distributing 
  the chatserver lobbys even more far)
- copy config folder to server
- create the folder certificateFolder and the file chatserver_temporary_friends.txt
  (hardcoded in chatserver.h!)
- start with ./retroshare-chatserver -c <configfolder>
- setup w2c as webinterface, see https://github.com/drbob/rs-w2c
- make sure everything is correct with your permissions!

