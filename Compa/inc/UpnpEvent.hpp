#ifndef COMPA_UPNPEVENT_HPP
#define COMPA_UPNPEVENT_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-08-13
// Also Copyright by other contributor as noted below.
/*!
 * \file
 * \brief Header file for UpnpEvent methods.
 * \authors Marcelo Roberto Jimenez, Ingo Höft
 */

#include <ixml.hpp>
#include <UpnpString.hpp>

/*!
 * UpnpEvent
 */
typedef struct s_UpnpEvent UpnpEvent;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*! Constructor */
UPNPLIB_API UpnpEvent* UpnpEvent_new();
/*! Destructor */
UPNPLIB_API void UpnpEvent_delete(UpnpEvent* p);
/*! Copy Constructor */
UPNPLIB_API UpnpEvent* UpnpEvent_dup(const UpnpEvent* p);
/*! Assignment operator */
UPNPLIB_API int UpnpEvent_assign(UpnpEvent* p, const UpnpEvent* q);

/*! UpnpEvent_get_EventKey */
UPNPLIB_API int UpnpEvent_get_EventKey(const UpnpEvent* p);
/*! UpnpEvent_set_EventKey */
UPNPLIB_API int UpnpEvent_set_EventKey(UpnpEvent* p, int n);

/*! UpnpEvent_get_ChangedVariables */
UPNPLIB_API IXML_Document* UpnpEvent_get_ChangedVariables(const UpnpEvent* p);
/*! UpnpEvent_set_ChangedVariables */
UPNPLIB_API int UpnpEvent_set_ChangedVariables(UpnpEvent* p, IXML_Document* n);

/*! UpnpEvent_get_SID */
UPNPLIB_API const UpnpString* UpnpEvent_get_SID(const UpnpEvent* p);
/*! UpnpEvent_set_SID */
UPNPLIB_API int UpnpEvent_set_SID(UpnpEvent* p, const UpnpString* s);
/*! UpnpEvent_get_SID_Length */
UPNPLIB_API size_t UpnpEvent_get_SID_Length(const UpnpEvent* p);
/*! UpnpEvent_get_SID_cstr */
UPNPLIB_API const char* UpnpEvent_get_SID_cstr(const UpnpEvent* p);
/*! UpnpEvent_strcpy_SID */
UPNPLIB_API int UpnpEvent_strcpy_SID(UpnpEvent* p, const char* s);
/*! UpnpEvent_strncpy_SID */
UPNPLIB_API int UpnpEvent_strncpy_SID(UpnpEvent* p, const char* s, size_t n);
/*! UpnpEvent_clear_SID */
UPNPLIB_API void UpnpEvent_clear_SID(UpnpEvent* p);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* COMPA_UPNPEVENT_HPP */
