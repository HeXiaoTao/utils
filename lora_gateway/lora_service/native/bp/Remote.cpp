/*
 * Bp native.
 *
 */

#define LOG_TAG "Remote"

#include <cutils/log.h>
#include <utils/Log.h>

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/PermissionCache.h>
#include <utils/String16.h>

#include <keystore/IKeystoreService.h>
#include <keystore/keystore.h> // for error codes

#include "Remote.h"

namespace android {

sp<Remote> Remote::gRemote;
Mutex Remote::gLock;

sp<IRemoteService> Remote::gRemoteService;
Mutex Remote::gLockRemote;

stateCallback_t Remote::gStateCallback;

//-----------------------------------------------------------

const sp<IRemoteService> Remote::get_remote_service()
{
	sp<IRemoteService> remote;
	{
		Mutex::Autolock _l(gLockRemote);
		if (gRemoteService == 0) {
			sp<IServiceManager> sm = defaultServiceManager();
			sp<IBinder> binder;
			do {
				binder = sm->getService(String16("hexiaotao.remote"));
				if (binder != 0)
				break;
				ALOGW("RemoteService not published, waiting...");
				usleep(500000); // 0.5 s
			} while (true);

			gRemoteService = interface_cast<IRemoteService>(binder);
			LOG_ALWAYS_FATAL_IF(gRemoteService == 0);
		}
		remote = gRemoteService;
	}

	return remote;
}

sp<Remote> Remote::getInstance()
{
	sp<Remote> r;
	{
		Mutex::Autolock _l(gLock);
		if (gRemote == 0) {
			gRemote = new Remote();
		}
		r = gRemote;
	}

	return r;
}

//-----------------------------------------------------------

Remote::Remote()
	: BnRemoteServiceClient()
{
	ALOGI("Remote()");
}

void Remote::onFirstRef()
{
	ALOGI("Remote onFirstRef()");

	const sp<IRemoteService>& remote = Remote::get_remote_service();
	if (remote != 0) {
		remote->registerCallback(this);
	}
}

Remote::~Remote()
{
	ALOGI("~Remote exit");
	const sp<IRemoteService>& remote = Remote::get_remote_service();
	if (remote != 0) {
		remote->unregisterCallback(this);
	}
}

void Remote::binderDied(const wp<IBinder>& who) {
	ALOGW("binderDied() %p, calling pid %d", who.unsafe_get(),
			IPCThreadState::self()->getCallingPid());
}

//remote service callback
status_t Remote::onStateChange(int state)
{
	ALOGI("onStateChange state=%d", state);

	if(gStateCallback != NULL) {
		gStateCallback(state);
	}

	return NO_ERROR;
}

//-----------------------------------------------------------

int Remote::callRemotePrint(const String16& message)
{
	const sp<IRemoteService>& remote = Remote::get_remote_service();

	if (remote == 0) return PERMISSION_DENIED;

	return remote->callRemotePrint(message);
}

int Remote::setListener(stateCallback_t cb)
{
	gStateCallback = cb;

	return 0;
}

};