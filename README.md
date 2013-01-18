retroshare-chatserver
=====================
This is a chat intro server for RetroShare, which (hopefully) deal with
the issues the current one from retroshare-nogui has.

How it works:
- download the git repo to retroshare-code/trunk/retroshare-chatserver
- change constants in chatserver.h to what you want to
- setup an account with retroshare-gui as usual, perhaps add some friends 
  which always are allowed to connect (leeching lobbys and distributing 
  the chatserver lobbys even more far)
- copy config folder to server
- goto this directory with cd
- create the directory "certs"
- create empty file "chatserver_temporary_friends.txt"
- start with ./retroshare-chatserver -c <configfolder>

If it segfaults -> you probably haven't created the folder "certs/" and 
"chatserver_temporary_friends.txt"

TODO:
====
- merge w2c, so that you get the corresponding webinterface, too?!?

