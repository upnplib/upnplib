// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-06-13

namespace upnplib {

void init_global_vars() {
    // initialize global variables with file scope for upnpapi.cpp
    memset(&virtualDirCallback, 0, sizeof(virtualDirCallback));
    pVirtualDirList = nullptr;
    // GlobalClientSubscribeMutex = {}; // mutex, must be initialized,
    // only used with gena.h
    GlobalHndRWLock = {}; // mutex, must be initialzed
    memset(&gTimerThread, 0xFF, sizeof(gTimerThread));
    gSDKInitMutex = PTHREAD_MUTEX_INITIALIZER;
    gUUIDMutex = {}; // mutex, must be initialzed
    // gSendThreadPool          // type ThreadPool must be initialized
    // gRecvThreadPool;         // type ThreadPool must be initialized
    // gMiniServerThreadPool;   // type ThreadPool must be initialized
    bWebServerState = WEB_SERVER_DISABLED;
    std::fill(std::begin(gIF_NAME), std::end(gIF_NAME), 0);
    std::fill(std::begin(gIF_IPV4), std::end(gIF_IPV4), 0);
    std::fill(std::begin(gIF_IPV4_NETMASK), std::end(gIF_IPV4_NETMASK), 0);
    std::fill(std::begin(gIF_IPV6), std::end(gIF_IPV6), 0);
    gIF_IPV6_PREFIX_LENGTH = 0;
    std::fill(std::begin(gIF_IPV6_ULA_GUA), std::end(gIF_IPV6_ULA_GUA), 0);
    gIF_IPV6_ULA_GUA_PREFIX_LENGTH = 0;
    gIF_INDEX = (unsigned)-1;
    LOCAL_PORT_V4 = 0;
    LOCAL_PORT_V6 = 0;
    LOCAL_PORT_V6_ULA_GUA = 0;
    for (int i = 0; i < NUM_HANDLE; i++)
        HandleTable[i] = {};
    g_maxContentLength = DEFAULT_SOAP_CONTENT_LENGTH;
    g_UpnpSdkEQMaxLen = MAX_SUBSCRIPTION_QUEUED_EVENTS;
    g_UpnpSdkEQMaxAge = MAX_SUBSCRIPTION_EVENT_AGE;
    UpnpSdkInit = 0;
    UpnpSdkClientRegistered = 0;
    UpnpSdkDeviceRegisteredV4 = 0;
    UpnpSdkDeviceregisteredV6 = 0;
#ifdef UPNP_HAVE_OPTSSDP
    strcpy(gUpnpSdkNLSuuid, "");
#endif /* UPNP_HAVE_OPTSSDP */
#ifdef UPNP_ENABLE_OPEN_SSL
    SSL_CTX* gSslCtx = nullptr;
#endif
}

} // namespace upnplib
