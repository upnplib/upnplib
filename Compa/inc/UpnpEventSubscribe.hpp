#ifndef COMPA_UPNPEVENTSUBSCRIBE_HPP
#define COMPA_UPNPEVENTSUBSCRIBE_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-01-28
/*!
 * \file
 * \brief Header file for UpnpEventSubscribe methods.
 * \authors Marcelo Roberto Jimenez, Ingo Höft
 */

#include <UpnpString.hpp>

/*!
 * UpnpEventSubscribe
 */
typedef struct s_UpnpEventSubscribe UpnpEventSubscribe;

/*! Constructor */
UPNPLIB_API UpnpEventSubscribe* UpnpEventSubscribe_new();
/*! Destructor */
UPNPLIB_API void UpnpEventSubscribe_delete(UpnpEventSubscribe* p);
/*! Copy Constructor */
UPNPLIB_API UpnpEventSubscribe*
UpnpEventSubscribe_dup(const UpnpEventSubscribe* p);
/*! Assignment operator */
UPNPLIB_API int UpnpEventSubscribe_assign(UpnpEventSubscribe* p,
                                          const UpnpEventSubscribe* q);

/*! UpnpEventSubscribe_get_ErrCode */
UPNPLIB_API int UpnpEventSubscribe_get_ErrCode(const UpnpEventSubscribe* p);
/*! UpnpEventSubscribe_set_ErrCode */
UPNPLIB_API int UpnpEventSubscribe_set_ErrCode(UpnpEventSubscribe* p, int n);

/*! UpnpEventSubscribe_get_TimeOut */
UPNPLIB_API int UpnpEventSubscribe_get_TimeOut(const UpnpEventSubscribe* p);
/*! UpnpEventSubscribe_set_TimeOut */
UPNPLIB_API int UpnpEventSubscribe_set_TimeOut(UpnpEventSubscribe* p, int n);

/*! UpnpEventSubscribe_get_SID */
UPNPLIB_API const UpnpString*
UpnpEventSubscribe_get_SID(const UpnpEventSubscribe* p);
/*! UpnpEventSubscribe_set_SID */
UPNPLIB_API int UpnpEventSubscribe_set_SID(UpnpEventSubscribe* p,
                                           const UpnpString* s);
/*! UpnpEventSubscribe_get_SID_Length */
UPNPLIB_API size_t
UpnpEventSubscribe_get_SID_Length(const UpnpEventSubscribe* p);
/*! UpnpEventSubscribe_get_SID_cstr */
UPNPLIB_API const char*
UpnpEventSubscribe_get_SID_cstr(const UpnpEventSubscribe* p);
/*! UpnpEventSubscribe_strcpy_SID */
UPNPLIB_API int UpnpEventSubscribe_strcpy_SID(UpnpEventSubscribe* p,
                                              const char* s);
/*! UpnpEventSubscribe_strncpy_SID */
UPNPLIB_API int UpnpEventSubscribe_strncpy_SID(UpnpEventSubscribe* p,
                                               const char* s, size_t n);
/*! UpnpEventSubscribe_clear_SID */
UPNPLIB_API void UpnpEventSubscribe_clear_SID(UpnpEventSubscribe* p);

/*! UpnpEventSubscribe_get_PublisherUrl */
UPNPLIB_API const UpnpString*
UpnpEventSubscribe_get_PublisherUrl(const UpnpEventSubscribe* p);
/*! UpnpEventSubscribe_set_PublisherUrl */
UPNPLIB_API int UpnpEventSubscribe_set_PublisherUrl(UpnpEventSubscribe* p,
                                                    const UpnpString* s);
/*! UpnpEventSubscribe_get_PublisherUrl_Length */
UPNPLIB_API size_t
UpnpEventSubscribe_get_PublisherUrl_Length(const UpnpEventSubscribe* p);
/*! UpnpEventSubscribe_get_PublisherUrl_cstr */
UPNPLIB_API const char*
UpnpEventSubscribe_get_PublisherUrl_cstr(const UpnpEventSubscribe* p);
/*! UpnpEventSubscribe_strcpy_PublisherUrl */
UPNPLIB_API int UpnpEventSubscribe_strcpy_PublisherUrl(UpnpEventSubscribe* p,
                                                       const char* s);
/*! UpnpEventSubscribe_strncpy_PublisherUrl */
UPNPLIB_API int UpnpEventSubscribe_strncpy_PublisherUrl(UpnpEventSubscribe* p,
                                                        const char* s,
                                                        size_t n);
/*! UpnpEventSubscribe_clear_PublisherUrl */
UPNPLIB_API void UpnpEventSubscribe_clear_PublisherUrl(UpnpEventSubscribe* p);

#endif /* COMPA_UPNPEVENTSUBSCRIBE_HPP*/
