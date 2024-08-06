#ifndef UPNPLIB_UPNPACTIONCOMPLETE_HPP
#define UPNPLIB_UPNPACTIONCOMPLETE_HPP
// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-08-13
// Also Copyright by other contributor as noted below.

/*!
 * \file
 *
 * \brief Header file for UpnpActionComplete methods.
 * \authors Marcelo Roberto Jimenez, Ingo Höft
 */
#include <stdlib.h> /* for size_t */

#include "UpnpGlobal.hpp" /* for EXPORT_SPEC */

#include "UpnpString.hpp"
#include "ixml.hpp"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * UpnpActionComplete
 */
typedef struct s_UpnpActionComplete UpnpActionComplete;

/*! Constructor */
EXPORT_SPEC UpnpActionComplete* UpnpActionComplete_new();
/*! Destructor */
EXPORT_SPEC void UpnpActionComplete_delete(UpnpActionComplete* p);
/*! Copy Constructor */
EXPORT_SPEC UpnpActionComplete*
UpnpActionComplete_dup(const UpnpActionComplete* p);
/*! Assignment operator */
EXPORT_SPEC int UpnpActionComplete_assign(UpnpActionComplete* p,
                                          const UpnpActionComplete* q);

/*! UpnpActionComplete_get_ErrCode */
EXPORT_SPEC int UpnpActionComplete_get_ErrCode(const UpnpActionComplete* p);
/*! UpnpActionComplete_set_ErrCode */
EXPORT_SPEC int UpnpActionComplete_set_ErrCode(UpnpActionComplete* p, int n);

/*! UpnpActionComplete_get_CtrlUrl */
EXPORT_SPEC const UpnpString*
UpnpActionComplete_get_CtrlUrl(const UpnpActionComplete* p);
/*! UpnpActionComplete_set_CtrlUrl */
EXPORT_SPEC int UpnpActionComplete_set_CtrlUrl(UpnpActionComplete* p,
                                               const UpnpString* s);
/*! UpnpActionComplete_get_CtrlUrl_Length */
EXPORT_SPEC size_t
UpnpActionComplete_get_CtrlUrl_Length(const UpnpActionComplete* p);
/*! UpnpActionComplete_get_CtrlUrl_cstr */
EXPORT_SPEC const char*
UpnpActionComplete_get_CtrlUrl_cstr(const UpnpActionComplete* p);
/*! UpnpActionComplete_strcpy_CtrlUrl */
EXPORT_SPEC int UpnpActionComplete_strcpy_CtrlUrl(UpnpActionComplete* p,
                                                  const char* s);
/*! UpnpActionComplete_strncpy_CtrlUrl */
EXPORT_SPEC int UpnpActionComplete_strncpy_CtrlUrl(UpnpActionComplete* p,
                                                   const char* s, size_t n);
/*! UpnpActionComplete_clear_CtrlUrl */
EXPORT_SPEC void UpnpActionComplete_clear_CtrlUrl(UpnpActionComplete* p);

/*! UpnpActionComplete_get_ActionRequest */
EXPORT_SPEC IXML_Document*
UpnpActionComplete_get_ActionRequest(const UpnpActionComplete* p);
/*! UpnpActionComplete_set_ActionRequest */
EXPORT_SPEC int UpnpActionComplete_set_ActionRequest(UpnpActionComplete* p,
                                                     IXML_Document* n);

/*! UpnpActionComplete_get_ActionResult */
EXPORT_SPEC IXML_Document*
UpnpActionComplete_get_ActionResult(const UpnpActionComplete* p);
/*! UpnpActionComplete_set_ActionResult */
EXPORT_SPEC int UpnpActionComplete_set_ActionResult(UpnpActionComplete* p,
                                                    IXML_Document* n);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UPNPLIB_UPNPACTIONCOMPLETE_HPP */
