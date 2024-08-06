#ifndef COMPA_UPNPSTATEVARCOMPLETE_HPP
#define COMPA_UPNPSTATEVARCOMPLETE_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-08-13
/*!
 * \file
 * \brief Header file for UpnpStateVarComplete methods.
 * \authors Marcelo Roberto Jimenez, Ingo Höft
 */

#include <UpnpString.hpp>
#include <ixml.hpp>

/// \cond
#include <cstdlib> /* for size_t */
/// \endcond

/*!
 * UpnpStateVarComplete
 */
typedef struct s_UpnpStateVarComplete UpnpStateVarComplete;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*! Constructor */
UPNPLIB_API UpnpStateVarComplete* UpnpStateVarComplete_new();
/*! Destructor */
UPNPLIB_API void UpnpStateVarComplete_delete(UpnpStateVarComplete* p);
/*! Copy Constructor */
UPNPLIB_API UpnpStateVarComplete*
UpnpStateVarComplete_dup(const UpnpStateVarComplete* p);
/*! Assignment operator */
UPNPLIB_API int UpnpStateVarComplete_assign(UpnpStateVarComplete* p,
                                            const UpnpStateVarComplete* q);

/*! UpnpStateVarComplete_get_ErrCode */
UPNPLIB_API int UpnpStateVarComplete_get_ErrCode(const UpnpStateVarComplete* p);
/*! UpnpStateVarComplete_set_ErrCode */
UPNPLIB_API int UpnpStateVarComplete_set_ErrCode(UpnpStateVarComplete* p,
                                                 int n);

/*! UpnpStateVarComplete_get_CtrlUrl */
UPNPLIB_API const UpnpString*
UpnpStateVarComplete_get_CtrlUrl(const UpnpStateVarComplete* p);
/*! UpnpStateVarComplete_set_CtrlUrl */
UPNPLIB_API int UpnpStateVarComplete_set_CtrlUrl(UpnpStateVarComplete* p,
                                                 const UpnpString* s);
/*! UpnpStateVarComplete_get_CtrlUrl_Length */
UPNPLIB_API size_t
UpnpStateVarComplete_get_CtrlUrl_Length(const UpnpStateVarComplete* p);
/*! UpnpStateVarComplete_get_CtrlUrl_cstr */
UPNPLIB_API const char*
UpnpStateVarComplete_get_CtrlUrl_cstr(const UpnpStateVarComplete* p);
/*! UpnpStateVarComplete_strcpy_CtrlUrl */
UPNPLIB_API int UpnpStateVarComplete_strcpy_CtrlUrl(UpnpStateVarComplete* p,
                                                    const char* s);
/*! UpnpStateVarComplete_strncpy_CtrlUrl */
UPNPLIB_API int UpnpStateVarComplete_strncpy_CtrlUrl(UpnpStateVarComplete* p,
                                                     const char* s, size_t n);
/*! UpnpStateVarComplete_clear_CtrlUrl */
UPNPLIB_API void UpnpStateVarComplete_clear_CtrlUrl(UpnpStateVarComplete* p);

/*! UpnpStateVarComplete_get_StateVarName */
UPNPLIB_API const UpnpString*
UpnpStateVarComplete_get_StateVarName(const UpnpStateVarComplete* p);
/*! UpnpStateVarComplete_set_StateVarName */
UPNPLIB_API int UpnpStateVarComplete_set_StateVarName(UpnpStateVarComplete* p,
                                                      const UpnpString* s);
/*! UpnpStateVarComplete_get_StateVarName_Length */
UPNPLIB_API size_t
UpnpStateVarComplete_get_StateVarName_Length(const UpnpStateVarComplete* p);
/*! UpnpStateVarComplete_get_StateVarName_cstr */
UPNPLIB_API const char*
UpnpStateVarComplete_get_StateVarName_cstr(const UpnpStateVarComplete* p);
/*! UpnpStateVarComplete_strcpy_StateVarName */
UPNPLIB_API int
UpnpStateVarComplete_strcpy_StateVarName(UpnpStateVarComplete* p,
                                         const char* s);
/*! UpnpStateVarComplete_strncpy_StateVarName */
UPNPLIB_API int
UpnpStateVarComplete_strncpy_StateVarName(UpnpStateVarComplete* p,
                                          const char* s, size_t n);
/*! UpnpStateVarComplete_clear_StateVarName */
UPNPLIB_API void
UpnpStateVarComplete_clear_StateVarName(UpnpStateVarComplete* p);

/*! UpnpStateVarComplete_get_CurrentVal */
UPNPLIB_API const DOMString
UpnpStateVarComplete_get_CurrentVal(const UpnpStateVarComplete* p);
/*! UpnpStateVarComplete_set_CurrentVal */
UPNPLIB_API int UpnpStateVarComplete_set_CurrentVal(UpnpStateVarComplete* p,
                                                    const DOMString s);
/*! UpnpStateVarComplete_get_CurrentVal_cstr */
UPNPLIB_API const char*
UpnpStateVarComplete_get_CurrentVal_cstr(const UpnpStateVarComplete* p);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* COMPA_UPNPSTATEVARCOMPLETE_HPP */
