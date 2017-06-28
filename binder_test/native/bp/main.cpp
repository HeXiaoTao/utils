/*
 * Bp native.
 *
 */

#define LOG_TAG "bp_native"

#include <cutils/log.h>
#include <utils/Log.h>

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/PermissionCache.h>
#include <utils/String16.h>

#include <keystore/IKeystoreService.h>
#include <keystore/keystore.h> // for error codes

#include "Remote.h"

int main() {
	int ret = -1;

	ALOGI("Starting " LOG_TAG);

	printf("Starting callRemotePrint\n");
	ret = android::Remote::callRemotePrint(android::String16("hello native call binder"));
	printf("Done return: %d\n", ret);

	ALOGI("Done");
	return 0;
}