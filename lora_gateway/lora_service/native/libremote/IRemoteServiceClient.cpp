/*
 * native common.
 *
 */

#define LOG_TAG "IRemoteServiceClient"
#include <utils/Log.h>

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>

#include <IRemoteServiceClient.h>

namespace android {

enum {
	ON_STATE_CHANGE = IBinder::FIRST_CALL_TRANSACTION,
};

class BpRemoteServiceClient : public BpInterface<IRemoteServiceClient>
{
public:
	BpRemoteServiceClient(const sp<IBinder>& impl)
		: BpInterface<IRemoteServiceClient>(impl)
	{
	}

	virtual status_t onStateChange(const int state)
	{
		Parcel data, reply;
		data.writeInterfaceToken(IRemoteServiceClient::getInterfaceDescriptor());
		data.writeInt32((int32_t)state);
		return remote()->transact(ON_STATE_CHANGE, data, &reply);
	}
};

IMPLEMENT_META_INTERFACE(RemoteServiceClient, "com.android.commands.bp.IRemoteServiceClient");

// ----------------------------------------------------------------------

status_t BnRemoteServiceClient::onTransact(
	uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
	switch (code) {
	case ON_STATE_CHANGE: {
		CHECK_INTERFACE(IRemoteServiceClient, data, reply);
		int state = data.readInt32();
		onStateChange(state);
		return NO_ERROR;
	} break;

	default:
		return BBinder::onTransact(code, data, reply, flags);
	}
}

};
