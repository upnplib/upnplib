#ifndef COMPA_UPNPACTIONCOMPLETE_HPP
#define COMPA_UPNPACTIONCOMPLETE_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-01-28
/*!
 * \file
 *
 * \brief Header file for UpnpActionComplete methods.
 * \authors Marcelo Roberto Jimenez, Ingo Höft
 */

#include <UpnpString.hpp>
#include <ixml.hpp>

/*! UpnpActionComplete */
typedef struct s_UpnpActionComplete UpnpActionComplete;

/*! Constructor */
UPNPLIB_API UpnpActionComplete* UpnpActionComplete_new();
/*! Destructor */
UPNPLIB_API void UpnpActionComplete_delete(UpnpActionComplete* p);
/*! Copy Constructor */
UPNPLIB_API UpnpActionComplete*
UpnpActionComplete_dup(const UpnpActionComplete* p);
/*! Assignment operator */
UPNPLIB_API int UpnpActionComplete_assign(UpnpActionComplete* p,
                                          const UpnpActionComplete* q);

/*! UpnpActionComplete_get_ErrCode */
UPNPLIB_API int UpnpActionComplete_get_ErrCode(const UpnpActionComplete* p);
/*! UpnpActionComplete_set_ErrCode */
UPNPLIB_API int UpnpActionComplete_set_ErrCode(UpnpActionComplete* p, int n);

/*! UpnpActionComplete_get_CtrlUrl */
UPNPLIB_API const UpnpString*
UpnpActionComplete_get_CtrlUrl(const UpnpActionComplete* p);
/*! UpnpActionComplete_set_CtrlUrl */
UPNPLIB_API int UpnpActionComplete_set_CtrlUrl(UpnpActionComplete* p,
                                               const UpnpString* s);
/*! UpnpActionComplete_get_CtrlUrl_Length */
UPNPLIB_API size_t
UpnpActionComplete_get_CtrlUrl_Length(const UpnpActionComplete* p);
/*! UpnpActionComplete_get_CtrlUrl_cstr */
UPNPLIB_API const char*
UpnpActionComplete_get_CtrlUrl_cstr(const UpnpActionComplete* p);
/*! UpnpActionComplete_strcpy_CtrlUrl */
UPNPLIB_API int UpnpActionComplete_strcpy_CtrlUrl(UpnpActionComplete* p,
                                                  const char* s);
/*! UpnpActionComplete_strncpy_CtrlUrl */
UPNPLIB_API int UpnpActionComplete_strncpy_CtrlUrl(UpnpActionComplete* p,
                                                   const char* s, size_t n);
/*! UpnpActionComplete_clear_CtrlUrl */
UPNPLIB_API void UpnpActionComplete_clear_CtrlUrl(UpnpActionComplete* p);

/*! UpnpActionComplete_get_ActionRequest */
UPNPLIB_API IXML_Document*
UpnpActionComplete_get_ActionRequest(const UpnpActionComplete* p);
/*! UpnpActionComplete_set_ActionRequest */
UPNPLIB_API int UpnpActionComplete_set_ActionRequest(UpnpActionComplete* p,
                                                     IXML_Document* n);

/*! UpnpActionComplete_get_ActionResult */
UPNPLIB_API IXML_Document*
UpnpActionComplete_get_ActionResult(const UpnpActionComplete* p);
/*! UpnpActionComplete_set_ActionResult */
UPNPLIB_API int UpnpActionComplete_set_ActionResult(UpnpActionComplete* p,
                                                    IXML_Document* n);
#endif /* COMPA_UPNPACTIONCOMPLETE_HPP */
