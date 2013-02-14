/*
 * MinimalNotify.h
 *
 *  Created on: Feb 14, 2013
 *      Author: klaus
 */

#ifndef MINIMALNOTIFY_H_
#define MINIMALNOTIFY_H_

#include <retroshare/rsiface.h>
#include <retroshare/rsturtle.h>
#include <string>


// discard all notifications, only the "askForPassword" needed for startup
class MinimalNotify : public NotifyBase
{
public:
	virtual bool askForPassword(const std::string& key_details, bool prev_is_bad, std::string& password);
};

#endif /* MINIMALNOTIFY_H_ */
