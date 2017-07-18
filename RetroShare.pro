!include("retroshare/retroshare.pri"): error("Could not include file retroshare/retroshare.pri")

TEMPLATE = subdirs

SUBDIRS += openpgpsdk
openpgpsdk.file = openpgpsdk/src/openpgpsdk.pro

SUBDIRS += libbitdht
libbitdht.file = libbitdht/src/libbitdht.pro

SUBDIRS += libretroshare
libretroshare.file = libretroshare/src/libretroshare.pro
libretroshare.depends = openpgpsdk libbitdht

SUBDIRS += retroshare_chatserver
retroshare_chatserver.file = retroshare-chatserver/src/chatserver.pro
retroshare_chatserver.depends = libretroshare
retroshare_chatserver.target = retroshare_chatserver
