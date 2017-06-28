/*
 * bp native.
 *
 */

#ifndef REMOTE_H
#define REMOTE_H

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include <utils/Mutex.h>
#include <utils/RefBase.h>
#include <utils/Errors.h>

#include <IRemoteService.h>

namespace android {

class Remote
{
public:
	static const sp<IRemoteService> get_remote_service();
	static int callRemotePrint(const String16& message);

private:
	Remote() ANDROID_API;
	virtual ~Remote();

	static sp<IRemoteService> gRemoteService;
	static Mutex gLockRemote;
};

};
#endif
