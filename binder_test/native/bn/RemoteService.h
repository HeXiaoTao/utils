/*
 * native common.
 *
 */

#ifndef REMOTESERVICE_H
#define REMOTESERVICE_H

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include <utils/RefBase.h>
#include <utils/Errors.h>
#include <binder/BinderService.h>

#include <IRemoteService.h>

namespace android {

class RemoteService :
	public BinderService<RemoteService>,
	public BnRemoteService,
	public IBinder::DeathRecipient
{
	friend class BinderService<RemoteService>;

	static const char *getServiceName() ANDROID_API { return "hexiaotao.remote"; }

public:
	//
	// IRemoteService interface.
	//
	virtual int callRemotePrint(const String16& message);
	virtual int registerCallback();
	virtual int unregisterCallback();

	virtual status_t onTransact(
		uint32_t code,
		const Parcel& data,
		Parcel* reply,
		uint32_t flags);

	virtual void binderDied(const wp<IBinder>& who);

	virtual void onFirstRef();

private:
	RemoteService() ANDROID_API;
	virtual ~RemoteService();
};

};
#endif
