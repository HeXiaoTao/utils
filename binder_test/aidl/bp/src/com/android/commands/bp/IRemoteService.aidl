package com.android.commands.bp;

interface IRemoteService
{
    int callRemotePrint(String message);

    int registerCallback();

    int unregisterCallback();
}
