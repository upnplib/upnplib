#ifndef COMPA_UPNPSUBSCRIPTIONREQUEST_HPP
#define COMPA_UPNPSUBSCRIPTIONREQUEST_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-01-28
/*!
 * \file
 * \brief Header file for UpnpSubscriptionRequest methods.
 * \authors Marcelo Roberto Jimenez, Ingo Höft
 */

#include <stdlib.h> /* for size_t */

#include "UpnpGlobal.hpp" /* for EXPORT_SPEC */

#include "UpnpString.hpp"

/*!
 * UpnpSubscriptionRequest
 */
typedef struct s_UpnpSubscriptionRequest UpnpSubscriptionRequest;

/*! Constructor */
EXPORT_SPEC UpnpSubscriptionRequest* UpnpSubscriptionRequest_new();
/*! Destructor */
EXPORT_SPEC void UpnpSubscriptionRequest_delete(UpnpSubscriptionRequest* p);
/*! Copy Constructor */
EXPORT_SPEC UpnpSubscriptionRequest*
UpnpSubscriptionRequest_dup(const UpnpSubscriptionRequest* p);
/*! Assignment operator */
EXPORT_SPEC int
UpnpSubscriptionRequest_assign(UpnpSubscriptionRequest* p,
                               const UpnpSubscriptionRequest* q);

/*! UpnpSubscriptionRequest_get_ServiceId */
EXPORT_SPEC const UpnpString*
UpnpSubscriptionRequest_get_ServiceId(const UpnpSubscriptionRequest* p);
/*! UpnpSubscriptionRequest_set_ServiceId */
EXPORT_SPEC int
UpnpSubscriptionRequest_set_ServiceId(UpnpSubscriptionRequest* p,
                                      const UpnpString* s);
/*! UpnpSubscriptionRequest_get_ServiceId_Length */
EXPORT_SPEC size_t
UpnpSubscriptionRequest_get_ServiceId_Length(const UpnpSubscriptionRequest* p);
/*! UpnpSubscriptionRequest_get_ServiceId_cstr */
EXPORT_SPEC const char*
UpnpSubscriptionRequest_get_ServiceId_cstr(const UpnpSubscriptionRequest* p);
/*! UpnpSubscriptionRequest_strcpy_ServiceId */
EXPORT_SPEC int
UpnpSubscriptionRequest_strcpy_ServiceId(UpnpSubscriptionRequest* p,
                                         const char* s);
/*! UpnpSubscriptionRequest_strncpy_ServiceId */
EXPORT_SPEC int
UpnpSubscriptionRequest_strncpy_ServiceId(UpnpSubscriptionRequest* p,
                                          const char* s, size_t n);
/*! UpnpSubscriptionRequest_clear_ServiceId */
EXPORT_SPEC void
UpnpSubscriptionRequest_clear_ServiceId(UpnpSubscriptionRequest* p);

/*! UpnpSubscriptionRequest_get_UDN */
EXPORT_SPEC const UpnpString*
UpnpSubscriptionRequest_get_UDN(const UpnpSubscriptionRequest* p);
/*! UpnpSubscriptionRequest_set_UDN */
EXPORT_SPEC int UpnpSubscriptionRequest_set_UDN(UpnpSubscriptionRequest* p,
                                                const UpnpString* s);
/*! UpnpSubscriptionRequest_get_UDN_Length */
EXPORT_SPEC size_t
UpnpSubscriptionRequest_get_UDN_Length(const UpnpSubscriptionRequest* p);
/*! UpnpSubscriptionRequest_get_UDN_cstr */
EXPORT_SPEC const char*
UpnpSubscriptionRequest_get_UDN_cstr(const UpnpSubscriptionRequest* p);
/*! UpnpSubscriptionRequest_strcpy_UDN */
EXPORT_SPEC int UpnpSubscriptionRequest_strcpy_UDN(UpnpSubscriptionRequest* p,
                                                   const char* s);
/*! UpnpSubscriptionRequest_strncpy_UDN */
EXPORT_SPEC int UpnpSubscriptionRequest_strncpy_UDN(UpnpSubscriptionRequest* p,
                                                    const char* s, size_t n);
/*! UpnpSubscriptionRequest_clear_UDN */
EXPORT_SPEC void UpnpSubscriptionRequest_clear_UDN(UpnpSubscriptionRequest* p);

/*! UpnpSubscriptionRequest_get_SID */
EXPORT_SPEC const UpnpString*
UpnpSubscriptionRequest_get_SID(const UpnpSubscriptionRequest* p);
/*! UpnpSubscriptionRequest_set_SID */
EXPORT_SPEC int UpnpSubscriptionRequest_set_SID(UpnpSubscriptionRequest* p,
                                                const UpnpString* s);
/*! UpnpSubscriptionRequest_get_SID_Length */
EXPORT_SPEC size_t
UpnpSubscriptionRequest_get_SID_Length(const UpnpSubscriptionRequest* p);
/*! UpnpSubscriptionRequest_get_SID_cstr */
EXPORT_SPEC const char*
UpnpSubscriptionRequest_get_SID_cstr(const UpnpSubscriptionRequest* p);
/*! UpnpSubscriptionRequest_strcpy_SID */
EXPORT_SPEC int UpnpSubscriptionRequest_strcpy_SID(UpnpSubscriptionRequest* p,
                                                   const char* s);
/*! UpnpSubscriptionRequest_strncpy_SID */
EXPORT_SPEC int UpnpSubscriptionRequest_strncpy_SID(UpnpSubscriptionRequest* p,
                                                    const char* s, size_t n);
/*! UpnpSubscriptionRequest_clear_SID */
EXPORT_SPEC void UpnpSubscriptionRequest_clear_SID(UpnpSubscriptionRequest* p);

#endif /* COMPA_UPNPSUBSCRIPTIONREQUEST_HPP */
