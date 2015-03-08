# warning: I have crippled this file, guess it'll work only on Linux 32/64bit now

TEMPLATE = app
TARGET = retroshare-chatserver
CONFIG += bitdht


QMAKE_CXXFLAGS += -std=c++0x

CONFIG += debug
debug {
        QMAKE_CFLAGS -= -O2
        QMAKE_CFLAGS += -O0
        QMAKE_CFLAGS += -g

        QMAKE_CXXFLAGS -= -O2
        QMAKE_CXXFLAGS += -O0
        QMAKE_CXXFLAGS += -g
}

################################# Linux ##########################################
linux-* {
	#CONFIG += version_detail_bash_script
	QMAKE_CXXFLAGS *= -D_FILE_OFFSET_BITS=64

	LIBS += ../../libretroshare/src/lib/libretroshare.a
	LIBS += ../../openpgpsdk/src/lib/libops.a -lbz2
	LIBS += -lssl -lupnp -lixml -lgnome-keyring
	LIBS *= -lcrypto -ldl -lz 

	SQLCIPHER_OK = $$system(pkg-config --exists sqlcipher && echo yes)
	isEmpty(SQLCIPHER_OK) {
# We need a explicit path here, to force using the home version of sqlite3 that really encrypts the database.

		exists(../../../lib/sqlcipher/.libs/libsqlcipher.a) {

			LIBS += ../../../lib/sqlcipher/.libs/libsqlcipher.a
			DEPENDPATH += ../../../lib/sqlcipher/src/
			INCLUDEPATH += ../../../lib/sqlcipher/src/
			DEPENDPATH += ../../../lib/sqlcipher/tsrc/
			INCLUDEPATH += ../../../lib/sqlcipher/tsrc/
		} else {
			message(libsqlcipher.a not found. Compilation will not use SQLCIPHER. Database will be unencrypted.)
			DEFINES *= NO_SQLCIPHER
			LIBS *= -lsqlite3
		}

	} else {
		LIBS += -lsqlcipher
	}
}

linux-g++ {
	OBJECTS_DIR = temp/linux-g++/obj
}

linux-g++-64 {
	OBJECTS_DIR = temp/linux-g++-64/obj
}
############################## Common stuff ######################################

# bitdht config
bitdht {
	LIBS += ../../libbitdht/src/lib/libbitdht.a
}

DEPENDPATH += ../../libretroshare/src
            
INCLUDEPATH += . ../../libretroshare/src

# Input
HEADERS +=  *.h 	
SOURCES +=  *.cpp 
	
	
