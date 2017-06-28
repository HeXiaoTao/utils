/*
 * AIDL Bp test.
 */

package com.android.commands.bp;

import android.os.Binder;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;

public class Bp {

    private static final String TAG = "bp-aidl";

    public static void main(String[] args) {
        (new Bp()).run(args);
    }

    void run(String[] args) {
        IRemoteService remote = IRemoteService.Stub.asInterface(ServiceManager.getService("hexiaotao.remote"));

        System.out.println("Bp run: " + remote);
        if(remote != null) {
            try {
				System.out.println("remote.callRemotePrint() ++ return: "
						+ remote.callRemotePrint("hello binder"));
            } catch(RemoteException e) {
                System.out.println("Bp run ERROR: " + e);
            }
        }
    }
}
