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
	String8 _message(message);

	ALOGI("callRemotePrint message:%s", _message.string());

	if(mCallback != NULL) {
		mCallback->onStateChange(88);
	}

	return 0;
}

int RemoteService::registerCallback(const sp<IRemoteServiceClient>& callback)
{
	ALOGI("registerCallback");

	if (mCallback != NULL && IInterface::asBinder(callback) != IInterface::asBinder(mCallback)) {
		IInterface::asBinder(mCallback)->unlinkToDeath(this);
	}

	IInterface::asBinder(callback)->linkToDeath(this);
	mCallback = callback;

	return 0;
}

int RemoteService::unregisterCallback(const sp<IRemoteServiceClient>& callback)
{
	ALOGI("unregisterCallback");

	if (mCallback != NULL && IInterface::asBinder(callback) == IInterface::asBinder(mCallback)) {
		IInterface::asBinder(mCallback)->unlinkToDeath(this);
		mCallback = NULL;
	}

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

	if (IInterface::asBinder(mCallback) == who) {
		mCallback = NULL;
	}
}

};
