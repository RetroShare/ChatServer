/*
 * MinimalNotify.cpp
 *
 *  Created on: Feb 14, 2013
 *      Author: klaus
 */

#include "MinimalNotify.h"

#include <string.h>
#include <unistd.h>


#ifdef WINDOWS_SYS
#include <conio.h>
#include <stdio.h>

#define PASS_MAX 512

char *getpass (const char *prompt)
{
    static char getpassbuf [PASS_MAX + 1];
    size_t i = 0;
    int c;

    if (prompt) {
        fputs (prompt, stderr);
        fflush (stderr);
    }

    for (;;) {
        c = _getch ();
        if (c == '\r') {
            getpassbuf [i] = '\0';
            break;
        }
        else if (i < PASS_MAX) {
            getpassbuf[i++] = c;
        }

        if (i >= PASS_MAX) {
            getpassbuf [i] = '\0';
            break;
        }
    }

    if (prompt) {
        fputs ("\r\n", stderr);
        fflush (stderr);
    }

    return getpassbuf;
}
#endif

bool NotifyTxt::askForPassword(const std::string& title, const std::string& /*question*/, bool /*prev_is_bad*/, std::string& password, bool& cancel)
{
	char *passwd = getpass(("Please enter GPG password for key " + title + ": ").c_str()) ;
	password = passwd;
	cancel = false;	
	return !password.empty();
}


