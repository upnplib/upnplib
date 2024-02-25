#ifdef COMPA_HAVE_DEVICE_SOAP

#ifndef COMPA_UPNPSTATEVARREQUEST_HPP
#define COMPA_UPNPSTATEVARREQUEST_HPP
// Copyright (C) 2024+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-02-28
/*!
 * \file
 * \brief Header file for UpnpStateVarRequest methods.
 * \authors Marcelo Roberto Jimenez, Ingo Höft
 */

#include <UpnpString.hpp>
#include <UpnpInet.hpp>
#include <ixml.hpp>

/*!
 * UpnpStateVarRequest
 */
typedef struct s_UpnpStateVarRequest UpnpStateVarRequest;

/*! Constructor */
UPNPLIB_API UpnpStateVarRequest* UpnpStateVarRequest_new();
/*! Destructor */
UPNPLIB_API void UpnpStateVarRequest_delete(UpnpStateVarRequest* p);
/*! Copy Constructor */
UPNPLIB_API UpnpStateVarRequest*
UpnpStateVarRequest_dup(const UpnpStateVarRequest* p);
/*! Assignment operator */
UPNPLIB_API int UpnpStateVarRequest_assign(UpnpStateVarRequest* p,
                                           const UpnpStateVarRequest* q);

/*! UpnpStateVarRequest_get_ErrCode */
UPNPLIB_API int UpnpStateVarRequest_get_ErrCode(const UpnpStateVarRequest* p);
/*! UpnpStateVarRequest_set_ErrCode */
UPNPLIB_API int UpnpStateVarRequest_set_ErrCode(UpnpStateVarRequest* p, int n);

/*! UpnpStateVarRequest_get_Socket */
UPNPLIB_API int UpnpStateVarRequest_get_Socket(const UpnpStateVarRequest* p);
/*! UpnpStateVarRequest_set_Socket */
UPNPLIB_API int UpnpStateVarRequest_set_Socket(UpnpStateVarRequest* p, int n);

/*! UpnpStateVarRequest_get_ErrStr */
UPNPLIB_API const UpnpString*
UpnpStateVarRequest_get_ErrStr(const UpnpStateVarRequest* p);
/*! UpnpStateVarRequest_set_ErrStr */
UPNPLIB_API int UpnpStateVarRequest_set_ErrStr(UpnpStateVarRequest* p,
                                               const UpnpString* s);
/*! UpnpStateVarRequest_get_ErrStr_Length */
UPNPLIB_API size_t
UpnpStateVarRequest_get_ErrStr_Length(const UpnpStateVarRequest* p);
/*! UpnpStateVarRequest_get_ErrStr_cstr */
UPNPLIB_API const char*
UpnpStateVarRequest_get_ErrStr_cstr(const UpnpStateVarRequest* p);
/*! UpnpStateVarRequest_strcpy_ErrStr */
UPNPLIB_API int UpnpStateVarRequest_strcpy_ErrStr(UpnpStateVarRequest* p,
                                                  const char* s);
/*! UpnpStateVarRequest_strncpy_ErrStr */
UPNPLIB_API int UpnpStateVarRequest_strncpy_ErrStr(UpnpStateVarRequest* p,
                                                   const char* s, size_t n);
/*! UpnpStateVarRequest_clear_ErrStr */
UPNPLIB_API void UpnpStateVarRequest_clear_ErrStr(UpnpStateVarRequest* p);

/*! UpnpStateVarRequest_get_DevUDN */
UPNPLIB_API const UpnpString*
UpnpStateVarRequest_get_DevUDN(const UpnpStateVarRequest* p);
/*! UpnpStateVarRequest_set_DevUDN */
UPNPLIB_API int UpnpStateVarRequest_set_DevUDN(UpnpStateVarRequest* p,
                                               const UpnpString* s);
/*! UpnpStateVarRequest_get_DevUDN_Length */
UPNPLIB_API size_t
UpnpStateVarRequest_get_DevUDN_Length(const UpnpStateVarRequest* p);
/*! UpnpStateVarRequest_get_DevUDN_cstr */
UPNPLIB_API const char*
UpnpStateVarRequest_get_DevUDN_cstr(const UpnpStateVarRequest* p);
/*! UpnpStateVarRequest_strcpy_DevUDN */
UPNPLIB_API int UpnpStateVarRequest_strcpy_DevUDN(UpnpStateVarRequest* p,
                                                  const char* s);
/*! UpnpStateVarRequest_strncpy_DevUDN */
UPNPLIB_API int UpnpStateVarRequest_strncpy_DevUDN(UpnpStateVarRequest* p,
                                                   const char* s, size_t n);
/*! UpnpStateVarRequest_clear_DevUDN */
UPNPLIB_API void UpnpStateVarRequest_clear_DevUDN(UpnpStateVarRequest* p);

/*! UpnpStateVarRequest_get_ServiceID */
UPNPLIB_API const UpnpString*
UpnpStateVarRequest_get_ServiceID(const UpnpStateVarRequest* p);
/*! UpnpStateVarRequest_set_ServiceID */
UPNPLIB_API int UpnpStateVarRequest_set_ServiceID(UpnpStateVarRequest* p,
                                                  const UpnpString* s);
/*! UpnpStateVarRequest_get_ServiceID_Length */
UPNPLIB_API size_t
UpnpStateVarRequest_get_ServiceID_Length(const UpnpStateVarRequest* p);
/*! UpnpStateVarRequest_get_ServiceID_cstr */
UPNPLIB_API const char*
UpnpStateVarRequest_get_ServiceID_cstr(const UpnpStateVarRequest* p);
/*! UpnpStateVarRequest_strcpy_ServiceID */
UPNPLIB_API int UpnpStateVarRequest_strcpy_ServiceID(UpnpStateVarRequest* p,
                                                     const char* s);
/*! UpnpStateVarRequest_strncpy_ServiceID */
UPNPLIB_API int UpnpStateVarRequest_strncpy_ServiceID(UpnpStateVarRequest* p,
                                                      const char* s, size_t n);
/*! UpnpStateVarRequest_clear_ServiceID */
UPNPLIB_API void UpnpStateVarRequest_clear_ServiceID(UpnpStateVarRequest* p);

/*! UpnpStateVarRequest_get_StateVarName */
UPNPLIB_API const UpnpString*
UpnpStateVarRequest_get_StateVarName(const UpnpStateVarRequest* p);
/*! UpnpStateVarRequest_set_StateVarName */
UPNPLIB_API int UpnpStateVarRequest_set_StateVarName(UpnpStateVarRequest* p,
                                                     const UpnpString* s);
/*! UpnpStateVarRequest_get_StateVarName_Length */
UPNPLIB_API size_t
UpnpStateVarRequest_get_StateVarName_Length(const UpnpStateVarRequest* p);
/*! UpnpStateVarRequest_get_StateVarName_cstr */
UPNPLIB_API const char*
UpnpStateVarRequest_get_StateVarName_cstr(const UpnpStateVarRequest* p);
/*! UpnpStateVarRequest_strcpy_StateVarName */
UPNPLIB_API int UpnpStateVarRequest_strcpy_StateVarName(UpnpStateVarRequest* p,
                                                        const char* s);
/*! UpnpStateVarRequest_strncpy_StateVarName */
UPNPLIB_API int UpnpStateVarRequest_strncpy_StateVarName(UpnpStateVarRequest* p,
                                                         const char* s,
                                                         size_t n);
/*! UpnpStateVarRequest_clear_StateVarName */
UPNPLIB_API void UpnpStateVarRequest_clear_StateVarName(UpnpStateVarRequest* p);

/*! UpnpStateVarRequest_get_CtrlPtIPAddr */
UPNPLIB_API const struct sockaddr_storage*
UpnpStateVarRequest_get_CtrlPtIPAddr(const UpnpStateVarRequest* p);
/*! UpnpStateVarRequest_get_CtrlPtIPAddr */
UPNPLIB_API int
UpnpStateVarRequest_set_CtrlPtIPAddr(UpnpStateVarRequest* p,
                                     const struct sockaddr_storage* buf);
/*! UpnpStateVarRequest_get_CtrlPtIPAddr */
UPNPLIB_API void UpnpStateVarRequest_clear_CtrlPtIPAddr(UpnpStateVarRequest* p);

/*! UpnpStateVarRequest_get_CurrentVal */
UPNPLIB_API const DOMString
UpnpStateVarRequest_get_CurrentVal(const UpnpStateVarRequest* p);
/*! UpnpStateVarRequest_set_CurrentVal */
UPNPLIB_API int UpnpStateVarRequest_set_CurrentVal(UpnpStateVarRequest* p,
                                                   const DOMString s);
/*! UpnpStateVarRequest_get_CurrentVal_cstr */
UPNPLIB_API const char*
UpnpStateVarRequest_get_CurrentVal_cstr(const UpnpStateVarRequest* p);

#endif // COMPA_UPNPSTATEVARREQUEST_HPP
#endif // COMPA_HAVE_DEVICE_SOAP
