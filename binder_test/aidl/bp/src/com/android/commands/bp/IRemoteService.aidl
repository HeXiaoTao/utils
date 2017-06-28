package com.android.commands.bp;

import com.android.commands.bp.IRemoteServiceClient;

interface IRemoteService
{
    int callRemotePrint(String message);

    int registerCallback(IRemoteServiceClient cb);

    int unregisterCallback(IRemoteServiceClient cb);
}
