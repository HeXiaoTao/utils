/*
 * Bn native remote.
 *
 */

#define LOG_TAG "RemoteService"

#include <cutils/log.h>
#include <utils/Log.h>

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/PermissionCache.h>
#include <utils/String16.h>

#include <keystore/IKeystoreService.h>
#include <keystore/keystore.h> // for error codes

#include "RemoteService.h"

namespace android {

RemoteService::RemoteService()
	: BnRemoteService()
{
	ALOGI("RemoteService()");
}

void RemoteService::onFirstRef()
{
	ALOGI("RemoteService onFirstRef()");
}

RemoteService::~RemoteService()
{
	ALOGI("~RemoteService exit");
}

int RemoteService::callRemotePrint(const String16& message)
{
	ALOGI("callRemotePrint message:%s", message.string());

	return 0;
}

int RemoteService::registerCallback()
{
	ALOGI("registerCallback");

	return 0;
}

int RemoteService::unregisterCallback()
{
	ALOGI("unregisterCallback");

	return 0;
}

status_t RemoteService::onTransact(uint32_t code,
	const Parcel& data, Parcel* reply, uint32_t flags)
{
	return BnRemoteService::onTransact(code, data, reply, flags);
}

void RemoteService::binderDied(const wp<IBinder>& who) {
	ALOGW("binderDied() %p, calling pid %d", who.unsafe_get(),
			IPCThreadState::self()->getCallingPid());
}

};
