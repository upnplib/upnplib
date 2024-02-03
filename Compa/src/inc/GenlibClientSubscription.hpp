#if defined(UPNP_HAVE_CLIENT) || defined(DOXYGEN_RUN)

#ifndef COMPA_GENLIB_CLIENTSUBSCRIPTION_HPP
#define COMPA_GENLIB_CLIENTSUBSCRIPTION_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-02-03
/*!
 * \file
 * \brief Header file for GenlibClientSubscription methods.
 * \authors Marcelo Roberto Jimenez, Ingo Höft
 */

#include <UpnpString.hpp>

/*!
 * GenlibClientSubscription
 */
typedef struct s_GenlibClientSubscription GenlibClientSubscription;

/*! Constructor */
UPNPLIB_API GenlibClientSubscription* GenlibClientSubscription_new();
/*! Destructor */
UPNPLIB_API void GenlibClientSubscription_delete(GenlibClientSubscription* p);
/*! Copy Constructor */
UPNPLIB_API GenlibClientSubscription*
GenlibClientSubscription_dup(const GenlibClientSubscription* p);
/*! Assignment operator */
UPNPLIB_API int
GenlibClientSubscription_assign(GenlibClientSubscription* p,
                                const GenlibClientSubscription* q);

/*! GenlibClientSubscription_get_RenewEventId */
UPNPLIB_API int
GenlibClientSubscription_get_RenewEventId(const GenlibClientSubscription* p);
/*! GenlibClientSubscription_set_RenewEventId */
UPNPLIB_API int
GenlibClientSubscription_set_RenewEventId(GenlibClientSubscription* p, int n);

/*! GenlibClientSubscription_get_SID */
UPNPLIB_API const UpnpString*
GenlibClientSubscription_get_SID(const GenlibClientSubscription* p);
/*! GenlibClientSubscription_set_SID */
UPNPLIB_API int GenlibClientSubscription_set_SID(GenlibClientSubscription* p,
                                                 const UpnpString* s);
/*! GenlibClientSubscription_get_SID_Length */
UPNPLIB_API size_t
GenlibClientSubscription_get_SID_Length(const GenlibClientSubscription* p);
/*! GenlibClientSubscription_get_SID_cstr */
UPNPLIB_API const char*
GenlibClientSubscription_get_SID_cstr(const GenlibClientSubscription* p);
/*! GenlibClientSubscription_strcpy_SID */
UPNPLIB_API int GenlibClientSubscription_strcpy_SID(GenlibClientSubscription* p,
                                                    const char* s);
/*! GenlibClientSubscription_strncpy_SID */
UPNPLIB_API int
GenlibClientSubscription_strncpy_SID(GenlibClientSubscription* p, const char* s,
                                     size_t n);
/*! GenlibClientSubscription_clear_SID */
UPNPLIB_API void
GenlibClientSubscription_clear_SID(GenlibClientSubscription* p);

/*! GenlibClientSubscription_get_ActualSID */
UPNPLIB_API const UpnpString*
GenlibClientSubscription_get_ActualSID(const GenlibClientSubscription* p);
/*! GenlibClientSubscription_set_ActualSID */
UPNPLIB_API int
GenlibClientSubscription_set_ActualSID(GenlibClientSubscription* p,
                                       const UpnpString* s);
/*! GenlibClientSubscription_get_ActualSID_Length */
UPNPLIB_API size_t GenlibClientSubscription_get_ActualSID_Length(
    const GenlibClientSubscription* p);
/*! GenlibClientSubscription_get_ActualSID_cstr */
UPNPLIB_API const char*
GenlibClientSubscription_get_ActualSID_cstr(const GenlibClientSubscription* p);
/*! GenlibClientSubscription_strcpy_ActualSID */
UPNPLIB_API int
GenlibClientSubscription_strcpy_ActualSID(GenlibClientSubscription* p,
                                          const char* s);
/*! GenlibClientSubscription_strncpy_ActualSID */
UPNPLIB_API int
GenlibClientSubscription_strncpy_ActualSID(GenlibClientSubscription* p,
                                           const char* s, size_t n);
/*! GenlibClientSubscription_clear_ActualSID */
UPNPLIB_API void
GenlibClientSubscription_clear_ActualSID(GenlibClientSubscription* p);

/*! GenlibClientSubscription_get_EventURL */
UPNPLIB_API const UpnpString*
GenlibClientSubscription_get_EventURL(const GenlibClientSubscription* p);
/*! GenlibClientSubscription_set_EventURL */
UPNPLIB_API int
GenlibClientSubscription_set_EventURL(GenlibClientSubscription* p,
                                      const UpnpString* s);
/*! GenlibClientSubscription_get_EventURL_Length */
UPNPLIB_API size_t
GenlibClientSubscription_get_EventURL_Length(const GenlibClientSubscription* p);
/*! GenlibClientSubscription_get_EventURL_cstr */
UPNPLIB_API const char*
GenlibClientSubscription_get_EventURL_cstr(const GenlibClientSubscription* p);
/*! GenlibClientSubscription_strcpy_EventURL */
UPNPLIB_API int
GenlibClientSubscription_strcpy_EventURL(GenlibClientSubscription* p,
                                         const char* s);
/*! GenlibClientSubscription_strncpy_EventURL */
UPNPLIB_API int
GenlibClientSubscription_strncpy_EventURL(GenlibClientSubscription* p,
                                          const char* s, size_t n);
/*! GenlibClientSubscription_clear_EventURL */
UPNPLIB_API void
GenlibClientSubscription_clear_EventURL(GenlibClientSubscription* p);

/*! GenlibClientSubscription_get_Next */
UPNPLIB_API GenlibClientSubscription*
GenlibClientSubscription_get_Next(const GenlibClientSubscription* p);
/*! GenlibClientSubscription_set_Next */
UPNPLIB_API int GenlibClientSubscription_set_Next(GenlibClientSubscription* p,
                                                  GenlibClientSubscription* n);

#endif // COMPA_GENLIB_CLIENTSUBSCRIPTION_HPP

#endif // UPNP_HAVE_CLIENT
