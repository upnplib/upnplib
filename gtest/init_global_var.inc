// include file to initialize global variables, usually included into the
// constructor of a TestSuite to avoid side effects between different tests.
// Get global list from the linker with: nm ../upnp/libupnp.a | grep ' [BDG] '
// Author: 2021-03-08 - Ingo Höft <Ingo@Hoeft-online.de>

//GlobalClientSubscribeMutex   // mutex, must be initialized, only used with gena.h
//GlobalHndRWLock              // mutex, must be initialzed
LOCAL_PORT_V4 = 0;
LOCAL_PORT_V6 = 0;
LOCAL_PORT_V6_ULA_GUA = 0;
//UpnpSdkClientRegistered = 0;    // not in header file, only file scope upnpapi.c
//UpnpSdkDeviceRegisteredV4 = 0;  // not in header file, only file scope upnpapi.c
//UpnpSdkDeviceregisteredV6 = 0;  // not in header file, only file scope upnpapi.c
//UpnpSdkInit = 0;                // not in header file, only scope file upnpapi.c
bWebServerState = (WebServerState)0;
gIF_INDEX = (unsigned)-1;
//*gIF_IPV4 = (const char&)"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
gIF_IPV4[0] = 0;
gIF_IPV4_NETMASK[0] = 0;
gIF_IPV6[0] = 0;
gIF_IPV6_PREFIX_LENGTH = 0;
gIF_IPV6_ULA_GUA[0] = 0;
gIF_IPV6_ULA_GUA_PREFIX_LENGTH = 0;
gIF_NAME[0] = 0;
//gMiniServerThreadPool  // type ThreadPool must be initialized
//gRecvThreadPool        // type ThreadPool must be initialized
//gSDKInitMutex          // mutex, must be initialzed
//gSendThreadPool        // type ThreadPool must be initialized
//gTimerThread           // must be initialized
//gUUIDMutex             // mutex, must be initialzed
gUpnpSdkNLSuuid[0] = 0;
g_UpnpSdkEQMaxAge = 30;
g_UpnpSdkEQMaxLen = 10;
g_maxContentLength = (const long unsigned int)16000;
pVirtualDirList = nullptr;
//virtualDirCallback     // structure must be initialized
//miniStopSockPort       // not in header file, only file scope upnpapi.c
//Http_Header_Names      // not in header file, only file scope upnpapi.c
//gDocumentRootDir       // struct, must be initialized
//ContentTypeHeader      // only used with soaplib.h
//gSsdpReqSocket4        // socket must be initialized, only used with ssdplib.h
//gSsdpReqSocket6        // socket must be initialized, only used with ssdplib.h
//ErrorMessages          // not in header file, only file scope upnpapi.c
