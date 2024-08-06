#ifndef COMPA_UPNPSUBSCRIPTIONREQUEST_HPP
#define COMPA_UPNPSUBSCRIPTIONREQUEST_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-08-13
/*!
 * \file
 * \brief Header file for UpnpSubscriptionRequest methods.
 * \authors Marcelo Roberto Jimenez, Ingo Höft
 */

#include "UpnpString.hpp"

/// \cond
#include <cstdlib> /* for size_t */
/// \endcond

/*!
 * UpnpSubscriptionRequest
 */
typedef struct s_UpnpSubscriptionRequest UpnpSubscriptionRequest;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*! Constructor */
UPNPLIB_API UpnpSubscriptionRequest* UpnpSubscriptionRequest_new();
/*! Destructor */
UPNPLIB_API void UpnpSubscriptionRequest_delete(UpnpSubscriptionRequest* p);
/*! Copy Constructor */
UPNPLIB_API UpnpSubscriptionRequest*
UpnpSubscriptionRequest_dup(const UpnpSubscriptionRequest* p);
/*! Assignment operator */
UPNPLIB_API int
UpnpSubscriptionRequest_assign(UpnpSubscriptionRequest* p,
                               const UpnpSubscriptionRequest* q);

/*! UpnpSubscriptionRequest_get_ServiceId */
UPNPLIB_API const UpnpString*
UpnpSubscriptionRequest_get_ServiceId(const UpnpSubscriptionRequest* p);
/*! UpnpSubscriptionRequest_set_ServiceId */
UPNPLIB_API int
UpnpSubscriptionRequest_set_ServiceId(UpnpSubscriptionRequest* p,
                                      const UpnpString* s);
/*! UpnpSubscriptionRequest_get_ServiceId_Length */
UPNPLIB_API size_t
UpnpSubscriptionRequest_get_ServiceId_Length(const UpnpSubscriptionRequest* p);
/*! UpnpSubscriptionRequest_get_ServiceId_cstr */
UPNPLIB_API const char*
UpnpSubscriptionRequest_get_ServiceId_cstr(const UpnpSubscriptionRequest* p);
/*! UpnpSubscriptionRequest_strcpy_ServiceId */
UPNPLIB_API int
UpnpSubscriptionRequest_strcpy_ServiceId(UpnpSubscriptionRequest* p,
                                         const char* s);
/*! UpnpSubscriptionRequest_strncpy_ServiceId */
UPNPLIB_API int
UpnpSubscriptionRequest_strncpy_ServiceId(UpnpSubscriptionRequest* p,
                                          const char* s, size_t n);
/*! UpnpSubscriptionRequest_clear_ServiceId */
UPNPLIB_API void
UpnpSubscriptionRequest_clear_ServiceId(UpnpSubscriptionRequest* p);

/*! UpnpSubscriptionRequest_get_UDN */
UPNPLIB_API const UpnpString*
UpnpSubscriptionRequest_get_UDN(const UpnpSubscriptionRequest* p);
/*! UpnpSubscriptionRequest_set_UDN */
UPNPLIB_API int UpnpSubscriptionRequest_set_UDN(UpnpSubscriptionRequest* p,
                                                const UpnpString* s);
/*! UpnpSubscriptionRequest_get_UDN_Length */
UPNPLIB_API size_t
UpnpSubscriptionRequest_get_UDN_Length(const UpnpSubscriptionRequest* p);
/*! UpnpSubscriptionRequest_get_UDN_cstr */
UPNPLIB_API const char*
UpnpSubscriptionRequest_get_UDN_cstr(const UpnpSubscriptionRequest* p);
/*! UpnpSubscriptionRequest_strcpy_UDN */
UPNPLIB_API int UpnpSubscriptionRequest_strcpy_UDN(UpnpSubscriptionRequest* p,
                                                   const char* s);
/*! UpnpSubscriptionRequest_strncpy_UDN */
UPNPLIB_API int UpnpSubscriptionRequest_strncpy_UDN(UpnpSubscriptionRequest* p,
                                                    const char* s, size_t n);
/*! UpnpSubscriptionRequest_clear_UDN */
UPNPLIB_API void UpnpSubscriptionRequest_clear_UDN(UpnpSubscriptionRequest* p);

/*! UpnpSubscriptionRequest_get_SID */
UPNPLIB_API const UpnpString*
UpnpSubscriptionRequest_get_SID(const UpnpSubscriptionRequest* p);
/*! UpnpSubscriptionRequest_set_SID */
UPNPLIB_API int UpnpSubscriptionRequest_set_SID(UpnpSubscriptionRequest* p,
                                                const UpnpString* s);
/*! UpnpSubscriptionRequest_get_SID_Length */
UPNPLIB_API size_t
UpnpSubscriptionRequest_get_SID_Length(const UpnpSubscriptionRequest* p);
/*! UpnpSubscriptionRequest_get_SID_cstr */
UPNPLIB_API const char*
UpnpSubscriptionRequest_get_SID_cstr(const UpnpSubscriptionRequest* p);
/*! UpnpSubscriptionRequest_strcpy_SID */
UPNPLIB_API int UpnpSubscriptionRequest_strcpy_SID(UpnpSubscriptionRequest* p,
                                                   const char* s);
/*! UpnpSubscriptionRequest_strncpy_SID */
UPNPLIB_API int UpnpSubscriptionRequest_strncpy_SID(UpnpSubscriptionRequest* p,
                                                    const char* s, size_t n);
/*! UpnpSubscriptionRequest_clear_SID */
UPNPLIB_API void UpnpSubscriptionRequest_clear_SID(UpnpSubscriptionRequest* p);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* COMPA_UPNPSUBSCRIPTIONREQUEST_HPP */
