#ifndef COMPA_UPNPACTIONREQUEST_HPP
#define COMPA_UPNPACTIONREQUEST_HPP
// Copyright (C) 2024+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-08-13
/*!
 * \file
 * \brief Header file for UpnpActionRequest methods.
 * \authors Marcelo Roberto Jimenez, Ingo Höft
 */

#include <UpnpString.hpp>
#include <ixml.hpp>

/*!
 * UpnpActionRequest
 */
typedef struct s_UpnpActionRequest UpnpActionRequest;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*! Constructor */
UPNPLIB_API UpnpActionRequest* UpnpActionRequest_new();
/*! Destructor */
UPNPLIB_API void UpnpActionRequest_delete(UpnpActionRequest* p);
/*! Copy Constructor */
UPNPLIB_API UpnpActionRequest*
UpnpActionRequest_dup(const UpnpActionRequest* p);
/*! Assignment operator */
UPNPLIB_API int UpnpActionRequest_assign(UpnpActionRequest* p,
                                         const UpnpActionRequest* q);

/*! UpnpActionRequest_get_ErrCode */
UPNPLIB_API int UpnpActionRequest_get_ErrCode(const UpnpActionRequest* p);
/*! UpnpActionRequest_set_ErrCode */
UPNPLIB_API int UpnpActionRequest_set_ErrCode(UpnpActionRequest* p, int n);

/*! UpnpActionRequest_get_Socket */
UPNPLIB_API int UpnpActionRequest_get_Socket(const UpnpActionRequest* p);
/*! UpnpActionRequest_set_Socket */
UPNPLIB_API int UpnpActionRequest_set_Socket(UpnpActionRequest* p, int n);

/*! UpnpActionRequest_get_ErrStr */
UPNPLIB_API const UpnpString*
UpnpActionRequest_get_ErrStr(const UpnpActionRequest* p);
/*! UpnpActionRequest_set_ErrStr */
UPNPLIB_API int UpnpActionRequest_set_ErrStr(UpnpActionRequest* p,
                                             const UpnpString* s);
/*! UpnpActionRequest_get_ErrStr_Length */
UPNPLIB_API size_t
UpnpActionRequest_get_ErrStr_Length(const UpnpActionRequest* p);
/*! UpnpActionRequest_get_ErrStr_cstr */
UPNPLIB_API const char*
UpnpActionRequest_get_ErrStr_cstr(const UpnpActionRequest* p);
/*! UpnpActionRequest_strcpy_ErrStr */
UPNPLIB_API int UpnpActionRequest_strcpy_ErrStr(UpnpActionRequest* p,
                                                const char* s);
/*! UpnpActionRequest_strncpy_ErrStr */
UPNPLIB_API int UpnpActionRequest_strncpy_ErrStr(UpnpActionRequest* p,
                                                 const char* s, size_t n);
/*! UpnpActionRequest_clear_ErrStr */
UPNPLIB_API void UpnpActionRequest_clear_ErrStr(UpnpActionRequest* p);

/*! UpnpActionRequest_get_ActionName */
UPNPLIB_API const UpnpString*
UpnpActionRequest_get_ActionName(const UpnpActionRequest* p);
/*! UpnpActionRequest_set_ActionName */
UPNPLIB_API int UpnpActionRequest_set_ActionName(UpnpActionRequest* p,
                                                 const UpnpString* s);
/*! UpnpActionRequest_get_ActionName_Length */
UPNPLIB_API size_t
UpnpActionRequest_get_ActionName_Length(const UpnpActionRequest* p);
/*! UpnpActionRequest_get_ActionName_cstr */
UPNPLIB_API const char*
UpnpActionRequest_get_ActionName_cstr(const UpnpActionRequest* p);
/*! UpnpActionRequest_strcpy_ActionName */
UPNPLIB_API int UpnpActionRequest_strcpy_ActionName(UpnpActionRequest* p,
                                                    const char* s);
/*! UpnpActionRequest_strncpy_ActionName */
UPNPLIB_API int UpnpActionRequest_strncpy_ActionName(UpnpActionRequest* p,
                                                     const char* s, size_t n);
/*! UpnpActionRequest_clear_ActionName */
UPNPLIB_API void UpnpActionRequest_clear_ActionName(UpnpActionRequest* p);

/*! UpnpActionRequest_get_DevUDN */
UPNPLIB_API const UpnpString*
UpnpActionRequest_get_DevUDN(const UpnpActionRequest* p);
/*! UpnpActionRequest_set_DevUDN */
UPNPLIB_API int UpnpActionRequest_set_DevUDN(UpnpActionRequest* p,
                                             const UpnpString* s);
/*! UpnpActionRequest_get_DevUDN_Length */
UPNPLIB_API size_t
UpnpActionRequest_get_DevUDN_Length(const UpnpActionRequest* p);
/*! UpnpActionRequest_get_DevUDN_cstr */
UPNPLIB_API const char*
UpnpActionRequest_get_DevUDN_cstr(const UpnpActionRequest* p);
/*! UpnpActionRequest_strcpy_DevUDN */
UPNPLIB_API int UpnpActionRequest_strcpy_DevUDN(UpnpActionRequest* p,
                                                const char* s);
/*! UpnpActionRequest_strncpy_DevUDN */
UPNPLIB_API int UpnpActionRequest_strncpy_DevUDN(UpnpActionRequest* p,
                                                 const char* s, size_t n);
/*! UpnpActionRequest_clear_DevUDN */
UPNPLIB_API void UpnpActionRequest_clear_DevUDN(UpnpActionRequest* p);

/*! UpnpActionRequest_get_ServiceID */
UPNPLIB_API const UpnpString*
UpnpActionRequest_get_ServiceID(const UpnpActionRequest* p);
/*! UpnpActionRequest_set_ServiceID */
UPNPLIB_API int UpnpActionRequest_set_ServiceID(UpnpActionRequest* p,
                                                const UpnpString* s);
/*! UpnpActionRequest_get_ServiceID_Length */
UPNPLIB_API size_t
UpnpActionRequest_get_ServiceID_Length(const UpnpActionRequest* p);
/*! UpnpActionRequest_get_ServiceID_cstr */
UPNPLIB_API const char*
UpnpActionRequest_get_ServiceID_cstr(const UpnpActionRequest* p);
/*! UpnpActionRequest_strcpy_ServiceID */
UPNPLIB_API int UpnpActionRequest_strcpy_ServiceID(UpnpActionRequest* p,
                                                   const char* s);
/*! UpnpActionRequest_strncpy_ServiceID */
UPNPLIB_API int UpnpActionRequest_strncpy_ServiceID(UpnpActionRequest* p,
                                                    const char* s, size_t n);
/*! UpnpActionRequest_clear_ServiceID */
UPNPLIB_API void UpnpActionRequest_clear_ServiceID(UpnpActionRequest* p);

/*! UpnpActionRequest_get_ActionRequest */
UPNPLIB_API IXML_Document*
UpnpActionRequest_get_ActionRequest(const UpnpActionRequest* p);
/*! UpnpActionRequest_set_ActionRequest */
UPNPLIB_API int UpnpActionRequest_set_ActionRequest(UpnpActionRequest* p,
                                                    IXML_Document* n);

/*! UpnpActionRequest_get_ActionResult */
UPNPLIB_API IXML_Document*
UpnpActionRequest_get_ActionResult(const UpnpActionRequest* p);
/*! UpnpActionRequest_set_ActionResult */
UPNPLIB_API int UpnpActionRequest_set_ActionResult(UpnpActionRequest* p,
                                                   IXML_Document* n);

/*! UpnpActionRequest_get_SoapHeader */
UPNPLIB_API IXML_Document*
UpnpActionRequest_get_SoapHeader(const UpnpActionRequest* p);
/*! UpnpActionRequest_set_SoapHeader */
UPNPLIB_API int UpnpActionRequest_set_SoapHeader(UpnpActionRequest* p,
                                                 IXML_Document* n);

/*! UpnpActionRequest_get_CtrlPtIPAddr */
UPNPLIB_API const struct sockaddr_storage*
UpnpActionRequest_get_CtrlPtIPAddr(const UpnpActionRequest* p);
/*! UpnpActionRequest_get_CtrlPtIPAddr */
UPNPLIB_API int
UpnpActionRequest_set_CtrlPtIPAddr(UpnpActionRequest* p,
                                   const struct sockaddr_storage* buf);
/*! UpnpActionRequest_get_CtrlPtIPAddr */
UPNPLIB_API void UpnpActionRequest_clear_CtrlPtIPAddr(UpnpActionRequest* p);

/*! UpnpActionRequest_get_Os */
UPNPLIB_API const UpnpString*
UpnpActionRequest_get_Os(const UpnpActionRequest* p);
/*! UpnpActionRequest_set_Os */
UPNPLIB_API int UpnpActionRequest_set_Os(UpnpActionRequest* p,
                                         const UpnpString* s);
/*! UpnpActionRequest_get_Os_Length */
UPNPLIB_API size_t UpnpActionRequest_get_Os_Length(const UpnpActionRequest* p);
/*! UpnpActionRequest_get_Os_cstr */
UPNPLIB_API const char*
UpnpActionRequest_get_Os_cstr(const UpnpActionRequest* p);
/*! UpnpActionRequest_strcpy_Os */
UPNPLIB_API int UpnpActionRequest_strcpy_Os(UpnpActionRequest* p,
                                            const char* s);
/*! UpnpActionRequest_strncpy_Os */
UPNPLIB_API int UpnpActionRequest_strncpy_Os(UpnpActionRequest* p,
                                             const char* s, size_t n);
/*! UpnpActionRequest_clear_Os */
UPNPLIB_API void UpnpActionRequest_clear_Os(UpnpActionRequest* p);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* COMPA_UPNPACTIONREQUEST_HPP */
