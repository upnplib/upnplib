// include file to initialize global variables, usually included into the
// constructor of a TestSuite to avoid side effects between different tests.
// Get global list from the linker with: nm ../upnp/libupnp.a | grep ' [BDG] '
// Author: 2021-03-08 - Ingo Höft <Ingo@Hoeft-online.de>

//miniStopSockPort       // not in header file, only file scope miniserver.c
//Http_Header_Names      // in httpparser.c and extern webserver.c
//gDocumentRootDir       // struct, must be initialized in webserver.c
//ContentTypeHeader      // used in soap_device.c, soap_ctrlpt.c, soap_common.c
//gSsdpReqSocket4        // socket, ssdp_server.c, ssdp_ctrlpt.c
//gSsdpReqSocket6        // socket, ssdp_server.c, ssdp_ctrlpt.c
//ErrorMessages          // only in upnptools.c
