#ifndef COMPA_UPNPDISCOVERY_HPP
#define COMPA_UPNPDISCOVERY_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-08-13
/*!
 * \file
 * \brief Header file for UpnpDiscovery methods.
 * \authors Marcelo Roberto Jimenez, Ingo Höft
 */

#include <UpnpString.hpp>

/*!
 * UpnpDiscovery
 */
typedef struct s_UpnpDiscovery UpnpDiscovery;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*! Constructor */
UPNPLIB_API UpnpDiscovery* UpnpDiscovery_new();
/*! Destructor */
UPNPLIB_API void UpnpDiscovery_delete(UpnpDiscovery* p);
/*! Copy Constructor */
UPNPLIB_API UpnpDiscovery* UpnpDiscovery_dup(const UpnpDiscovery* p);
/*! Assignment operator */
UPNPLIB_API int UpnpDiscovery_assign(UpnpDiscovery* p, const UpnpDiscovery* q);

/*! UpnpDiscovery_get_ErrCode */
UPNPLIB_API int UpnpDiscovery_get_ErrCode(const UpnpDiscovery* p);
/*! UpnpDiscovery_set_ErrCode */
UPNPLIB_API int UpnpDiscovery_set_ErrCode(UpnpDiscovery* p, int n);

/*! UpnpDiscovery_get_Expires */
UPNPLIB_API int UpnpDiscovery_get_Expires(const UpnpDiscovery* p);
/*! UpnpDiscovery_set_Expires */
UPNPLIB_API int UpnpDiscovery_set_Expires(UpnpDiscovery* p, int n);

/*! UpnpDiscovery_get_DeviceID */
UPNPLIB_API const UpnpString*
UpnpDiscovery_get_DeviceID(const UpnpDiscovery* p);
/*! UpnpDiscovery_set_DeviceID */
UPNPLIB_API int UpnpDiscovery_set_DeviceID(UpnpDiscovery* p,
                                           const UpnpString* s);
/*! UpnpDiscovery_get_DeviceID_Length */
UPNPLIB_API size_t UpnpDiscovery_get_DeviceID_Length(const UpnpDiscovery* p);
/*! UpnpDiscovery_get_DeviceID_cstr */
UPNPLIB_API const char* UpnpDiscovery_get_DeviceID_cstr(const UpnpDiscovery* p);
/*! UpnpDiscovery_strcpy_DeviceID */
UPNPLIB_API int UpnpDiscovery_strcpy_DeviceID(UpnpDiscovery* p, const char* s);
/*! UpnpDiscovery_strncpy_DeviceID */
UPNPLIB_API int UpnpDiscovery_strncpy_DeviceID(UpnpDiscovery* p, const char* s,
                                               size_t n);
/*! UpnpDiscovery_clear_DeviceID */
UPNPLIB_API void UpnpDiscovery_clear_DeviceID(UpnpDiscovery* p);

/*! UpnpDiscovery_get_DeviceType */
UPNPLIB_API const UpnpString*
UpnpDiscovery_get_DeviceType(const UpnpDiscovery* p);
/*! UpnpDiscovery_set_DeviceType */
UPNPLIB_API int UpnpDiscovery_set_DeviceType(UpnpDiscovery* p,
                                             const UpnpString* s);
/*! UpnpDiscovery_get_DeviceType_Length */
UPNPLIB_API size_t UpnpDiscovery_get_DeviceType_Length(const UpnpDiscovery* p);
/*! UpnpDiscovery_get_DeviceType_cstr */
UPNPLIB_API const char*
UpnpDiscovery_get_DeviceType_cstr(const UpnpDiscovery* p);
/*! UpnpDiscovery_strcpy_DeviceType */
UPNPLIB_API int UpnpDiscovery_strcpy_DeviceType(UpnpDiscovery* p,
                                                const char* s);
/*! UpnpDiscovery_strncpy_DeviceType */
UPNPLIB_API int UpnpDiscovery_strncpy_DeviceType(UpnpDiscovery* p,
                                                 const char* s, size_t n);
/*! UpnpDiscovery_clear_DeviceType */
UPNPLIB_API void UpnpDiscovery_clear_DeviceType(UpnpDiscovery* p);

/*! UpnpDiscovery_get_ServiceType */
UPNPLIB_API const UpnpString*
UpnpDiscovery_get_ServiceType(const UpnpDiscovery* p);
/*! UpnpDiscovery_set_ServiceType */
UPNPLIB_API int UpnpDiscovery_set_ServiceType(UpnpDiscovery* p,
                                              const UpnpString* s);
/*! UpnpDiscovery_get_ServiceType_Length */
UPNPLIB_API size_t UpnpDiscovery_get_ServiceType_Length(const UpnpDiscovery* p);
/*! UpnpDiscovery_get_ServiceType_cstr */
UPNPLIB_API const char*
UpnpDiscovery_get_ServiceType_cstr(const UpnpDiscovery* p);
/*! UpnpDiscovery_strcpy_ServiceType */
UPNPLIB_API int UpnpDiscovery_strcpy_ServiceType(UpnpDiscovery* p,
                                                 const char* s);
/*! UpnpDiscovery_strncpy_ServiceType */
UPNPLIB_API int UpnpDiscovery_strncpy_ServiceType(UpnpDiscovery* p,
                                                  const char* s, size_t n);
/*! UpnpDiscovery_clear_ServiceType */
UPNPLIB_API void UpnpDiscovery_clear_ServiceType(UpnpDiscovery* p);

/*! UpnpDiscovery_get_ServiceVer */
UPNPLIB_API const UpnpString*
UpnpDiscovery_get_ServiceVer(const UpnpDiscovery* p);
/*! UpnpDiscovery_set_ServiceVer */
UPNPLIB_API int UpnpDiscovery_set_ServiceVer(UpnpDiscovery* p,
                                             const UpnpString* s);
/*! UpnpDiscovery_get_ServiceVer_Length */
UPNPLIB_API size_t UpnpDiscovery_get_ServiceVer_Length(const UpnpDiscovery* p);
/*! UpnpDiscovery_get_ServiceVer_cstr */
UPNPLIB_API const char*
UpnpDiscovery_get_ServiceVer_cstr(const UpnpDiscovery* p);
/*! UpnpDiscovery_strcpy_ServiceVer */
UPNPLIB_API int UpnpDiscovery_strcpy_ServiceVer(UpnpDiscovery* p,
                                                const char* s);
/*! UpnpDiscovery_strncpy_ServiceVer */
UPNPLIB_API int UpnpDiscovery_strncpy_ServiceVer(UpnpDiscovery* p,
                                                 const char* s, size_t n);
/*! UpnpDiscovery_clear_ServiceVer */
UPNPLIB_API void UpnpDiscovery_clear_ServiceVer(UpnpDiscovery* p);

/*! UpnpDiscovery_get_Location */
UPNPLIB_API const UpnpString*
UpnpDiscovery_get_Location(const UpnpDiscovery* p);
/*! UpnpDiscovery_set_Location */
UPNPLIB_API int UpnpDiscovery_set_Location(UpnpDiscovery* p,
                                           const UpnpString* s);
/*! UpnpDiscovery_get_Location_Length */
UPNPLIB_API size_t UpnpDiscovery_get_Location_Length(const UpnpDiscovery* p);
/*! UpnpDiscovery_get_Location_cstr */
UPNPLIB_API const char* UpnpDiscovery_get_Location_cstr(const UpnpDiscovery* p);
/*! UpnpDiscovery_strcpy_Location */
UPNPLIB_API int UpnpDiscovery_strcpy_Location(UpnpDiscovery* p, const char* s);
/*! UpnpDiscovery_strncpy_Location */
UPNPLIB_API int UpnpDiscovery_strncpy_Location(UpnpDiscovery* p, const char* s,
                                               size_t n);
/*! UpnpDiscovery_clear_Location */
UPNPLIB_API void UpnpDiscovery_clear_Location(UpnpDiscovery* p);

/*! UpnpDiscovery_get_Os */
UPNPLIB_API const UpnpString* UpnpDiscovery_get_Os(const UpnpDiscovery* p);
/*! UpnpDiscovery_set_Os */
UPNPLIB_API int UpnpDiscovery_set_Os(UpnpDiscovery* p, const UpnpString* s);
/*! UpnpDiscovery_get_Os_Length */
UPNPLIB_API size_t UpnpDiscovery_get_Os_Length(const UpnpDiscovery* p);
/*! UpnpDiscovery_get_Os_cstr */
UPNPLIB_API const char* UpnpDiscovery_get_Os_cstr(const UpnpDiscovery* p);
/*! UpnpDiscovery_strcpy_Os */
UPNPLIB_API int UpnpDiscovery_strcpy_Os(UpnpDiscovery* p, const char* s);
/*! UpnpDiscovery_strncpy_Os */
UPNPLIB_API int UpnpDiscovery_strncpy_Os(UpnpDiscovery* p, const char* s,
                                         size_t n);
/*! UpnpDiscovery_clear_Os */
UPNPLIB_API void UpnpDiscovery_clear_Os(UpnpDiscovery* p);

/*! UpnpDiscovery_get_Date */
UPNPLIB_API const UpnpString* UpnpDiscovery_get_Date(const UpnpDiscovery* p);
/*! UpnpDiscovery_set_Date */
UPNPLIB_API int UpnpDiscovery_set_Date(UpnpDiscovery* p, const UpnpString* s);
/*! UpnpDiscovery_get_Date_Length */
UPNPLIB_API size_t UpnpDiscovery_get_Date_Length(const UpnpDiscovery* p);
/*! UpnpDiscovery_get_Date_cstr */
UPNPLIB_API const char* UpnpDiscovery_get_Date_cstr(const UpnpDiscovery* p);
/*! UpnpDiscovery_strcpy_Date */
UPNPLIB_API int UpnpDiscovery_strcpy_Date(UpnpDiscovery* p, const char* s);
/*! UpnpDiscovery_strncpy_Date */
UPNPLIB_API int UpnpDiscovery_strncpy_Date(UpnpDiscovery* p, const char* s,
                                           size_t n);
/*! UpnpDiscovery_clear_Date */
UPNPLIB_API void UpnpDiscovery_clear_Date(UpnpDiscovery* p);

/*! UpnpDiscovery_get_Ext */
UPNPLIB_API const UpnpString* UpnpDiscovery_get_Ext(const UpnpDiscovery* p);
/*! UpnpDiscovery_set_Ext */
UPNPLIB_API int UpnpDiscovery_set_Ext(UpnpDiscovery* p, const UpnpString* s);
/*! UpnpDiscovery_get_Ext_Length */
UPNPLIB_API size_t UpnpDiscovery_get_Ext_Length(const UpnpDiscovery* p);
/*! UpnpDiscovery_get_Ext_cstr */
UPNPLIB_API const char* UpnpDiscovery_get_Ext_cstr(const UpnpDiscovery* p);
/*! UpnpDiscovery_strcpy_Ext */
UPNPLIB_API int UpnpDiscovery_strcpy_Ext(UpnpDiscovery* p, const char* s);
/*! UpnpDiscovery_strncpy_Ext */
UPNPLIB_API int UpnpDiscovery_strncpy_Ext(UpnpDiscovery* p, const char* s,
                                          size_t n);
/*! UpnpDiscovery_clear_Ext */
UPNPLIB_API void UpnpDiscovery_clear_Ext(UpnpDiscovery* p);

/*! UpnpDiscovery_get_DestAddr */
UPNPLIB_API const struct sockaddr_storage*
UpnpDiscovery_get_DestAddr(const UpnpDiscovery* p);
/*! UpnpDiscovery_get_DestAddr */
UPNPLIB_API int UpnpDiscovery_set_DestAddr(UpnpDiscovery* p,
                                           const struct sockaddr_storage* buf);
/*! UpnpDiscovery_get_DestAddr */
UPNPLIB_API void UpnpDiscovery_clear_DestAddr(UpnpDiscovery* p);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* COMPA_UPNPDISCOVERY_HPP */
