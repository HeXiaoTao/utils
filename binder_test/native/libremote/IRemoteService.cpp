/*
 * native common.
 *
 */

#define LOG_TAG "IRemoteService"
#include <utils/Log.h>

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>

#include <IRemoteService.h>
#include <IRemoteServiceClient.h>

namespace android {

enum {
	CALL_REMOTE_PRINT = IBinder::FIRST_CALL_TRANSACTION,
	REGISTER_CALLBACK,
	UNREGISTER_CALLBACK,
};

class BpRemoteService : public BpInterface<IRemoteService>
{
public:
	BpRemoteService(const sp<IBinder>& impl)
		: BpInterface<IRemoteService>(impl)
	{
	}

	virtual int callRemotePrint(const String16& message)
	{
		Parcel data, reply;
		data.writeInterfaceToken(IRemoteService::getInterfaceDescriptor());
		data.writeString16(message);
		status_t status = remote()->transact(CALL_REMOTE_PRINT, data, &reply);
		if (status != NO_ERROR) {
			return -1;
		}

		return (int)(reply.readInt32());
	}

	virtual int registerCallback(const sp<IRemoteServiceClient>& callback)
	{
		Parcel data, reply;
		data.writeInterfaceToken(IRemoteService::getInterfaceDescriptor());
		data.writeStrongBinder(IInterface::asBinder(callback));
		status_t status = remote()->transact(REGISTER_CALLBACK, data, &reply);
		if (status != NO_ERROR) {
			return -1;
		}

		return (int)reply.readInt32();
	}

	virtual int unregisterCallback(const sp<IRemoteServiceClient>& callback)
	{
		Parcel data, reply;
		data.writeInterfaceToken(IRemoteService::getInterfaceDescriptor());
		data.writeStrongBinder(IInterface::asBinder(callback));
		status_t status = remote()->transact(UNREGISTER_CALLBACK, data, &reply);
		if (status != NO_ERROR) {
			return -1;
		}

		return (int)reply.readInt32();
	}
};

IMPLEMENT_META_INTERFACE(RemoteService, "com.android.commands.bp.IRemoteService");

// ----------------------------------------------------------------------

status_t BnRemoteService::onTransact(
	uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
	switch (code) {
	case CALL_REMOTE_PRINT: {
		CHECK_INTERFACE(IRemoteService, data, reply);
		const String16 message = data.readString16();
		reply->writeInt32(static_cast<uint32_t> (callRemotePrint(message)));
		return NO_ERROR;
	} break;

	case REGISTER_CALLBACK: {
		CHECK_INTERFACE(IRemoteService, data, reply);
		sp<IRemoteServiceClient> client = interface_cast<IRemoteServiceClient>(data.readStrongBinder());
		reply->writeInt32(static_cast <uint32_t>(registerCallback(client)));
		return NO_ERROR;
	} break;

	case UNREGISTER_CALLBACK: {
		CHECK_INTERFACE(IRemoteService, data, reply);
		sp<IRemoteServiceClient> client = interface_cast<IRemoteServiceClient>(data.readStrongBinder());
		reply->writeInt32(static_cast <uint32_t>(unregisterCallback(client)));
		return NO_ERROR;
	} break;

	default:
		return BBinder::onTransact(code, data, reply, flags);
	}
}

};
