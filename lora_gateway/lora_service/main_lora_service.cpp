/*
 * Bn native remote.
 *
 */

#define LOG_TAG "main_lora_service"

#include <cutils/log.h>
#include <utils/Log.h>

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/PermissionCache.h>
#include <utils/String16.h>

#include <keystore/IKeystoreService.h>
#include <keystore/keystore.h> // for error codes

#include "native/bn/RemoteService.h"

#include "lora_mac.h"

using namespace android;

int main() {
	ALOGI("Starting " LOG_TAG);

	if(!lora_mac_init()) {
		ALOGE("lora_mac_init error !!!!!!!!");
		return -1;
	}

	sp<ProcessState> proc(ProcessState::self());
	RemoteService::instantiate();

	/*
	* We're the only thread in existence, so we're just going to process
	* Binder transaction as a single-threaded program.
	*/
	ProcessState::self()->startThreadPool();
	IPCThreadState::self()->joinThreadPool();
	ALOGI("Done");
	return 0;
}
