/*
 * Bn native remote.
 *
 */

#define LOG_TAG "bn_native"

#include <cutils/log.h>
#include <utils/Log.h>

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/PermissionCache.h>
#include <utils/String16.h>

#include <keystore/IKeystoreService.h>
#include <keystore/keystore.h> // for error codes

#include "RemoteService.h"

int main() {
	ALOGI("Starting " LOG_TAG);

	android::RemoteService::instantiate();

	/*
	* We're the only thread in existence, so we're just going to process
	* Binder transaction as a single-threaded program.
	*/
	android::IPCThreadState::self()->joinThreadPool();
	ALOGI("Done");
	return 0;
}
