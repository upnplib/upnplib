#ifndef UPNPLIB_UPNPEXTRAHEADERS_HPP
#define UPNPLIB_UPNPEXTRAHEADERS_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-01-26
/*!
 * \file
 * \brief Header file for UpnpExtraHeaders methods.
 * \author Marcelo Roberto Jimenez
 */

#include <UpnpString.hpp>
#include <ixml.hpp>
#include <list.hpp>

/*!
 * UpnpExtraHeaders
 */
typedef struct s_UpnpExtraHeaders UpnpExtraHeaders;

/*! Constructor */
UPNPLIB_API UpnpExtraHeaders* UpnpExtraHeaders_new();
/*! Destructor */
UPNPLIB_API void UpnpExtraHeaders_delete(UpnpExtraHeaders* p);
/*! Copy Constructor */
// EXPORT_SPEC UpnpExtraHeaders *UpnpExtraHeaders_dup(const UpnpExtraHeaders
// *p);
/*! Assignment operator */
// EXPORT_SPEC int UpnpExtraHeaders_assign(
//        UpnpExtraHeaders *p, const UpnpExtraHeaders *q);

/*! UpnpExtraHeaders_get_node */
UPNPLIB_API const UpnpListHead*
UpnpExtraHeaders_get_node(const UpnpExtraHeaders* p);
/*! UpnpExtraHeaders_set_node */
UPNPLIB_API int UpnpExtraHeaders_set_node(UpnpExtraHeaders* p,
                                          const UpnpListHead* q);
/*! UpnpExtraHeaders_add_to_list_node */
// EXPORT_SPEC void UpnpExtraHeaders_add_to_list_node(
//        UpnpExtraHeaders *p, UpnpListHead *head);

/*! UpnpExtraHeaders_get_name */
UPNPLIB_API const UpnpString*
UpnpExtraHeaders_get_name(const UpnpExtraHeaders* p);
/*! UpnpExtraHeaders_set_name */
UPNPLIB_API int UpnpExtraHeaders_set_name(UpnpExtraHeaders* p,
                                          const UpnpString* s);
/*! UpnpExtraHeaders_get_name_Length */
// EXPORT_SPEC size_t UpnpExtraHeaders_get_name_Length(const UpnpExtraHeaders
// *p);
/*! UpnpExtraHeaders_get_name_cstr */
// EXPORT_SPEC const char *UpnpExtraHeaders_get_name_cstr(
//        const UpnpExtraHeaders *p);
/*! UpnpExtraHeaders_strcpy_name */
// EXPORT_SPEC int UpnpExtraHeaders_strcpy_name(
//        UpnpExtraHeaders *p, const char *s);
/*! UpnpExtraHeaders_strncpy_name */
UPNPLIB_API int UpnpExtraHeaders_strncpy_name(UpnpExtraHeaders* p,
                                              const char* s, size_t n);
/*! UpnpExtraHeaders_clear_name */
// EXPORT_SPEC void UpnpExtraHeaders_clear_name(UpnpExtraHeaders *p);

/*! UpnpExtraHeaders_get_value */
UPNPLIB_API const UpnpString*
UpnpExtraHeaders_get_value(const UpnpExtraHeaders* p);
/*! UpnpExtraHeaders_set_value */
UPNPLIB_API int UpnpExtraHeaders_set_value(UpnpExtraHeaders* p,
                                           const UpnpString* s);
/*! UpnpExtraHeaders_get_value_Length */
// EXPORT_SPEC size_t UpnpExtraHeaders_get_value_Length(const UpnpExtraHeaders
// *p);
/*! UpnpExtraHeaders_get_value_cstr */
// EXPORT_SPEC const char *UpnpExtraHeaders_get_value_cstr(
//        const UpnpExtraHeaders *p);
/*! UpnpExtraHeaders_strcpy_value */
// EXPORT_SPEC int UpnpExtraHeaders_strcpy_value(
//        UpnpExtraHeaders *p, const char *s);
/*! UpnpExtraHeaders_strncpy_value */
UPNPLIB_API int UpnpExtraHeaders_strncpy_value(UpnpExtraHeaders* p,
                                               const char* s, size_t n);
/*! UpnpExtraHeaders_clear_value */
// EXPORT_SPEC void UpnpExtraHeaders_clear_value(UpnpExtraHeaders *p);

/*! UpnpExtraHeaders_get_resp */
UPNPLIB_API const DOMString
UpnpExtraHeaders_get_resp(const UpnpExtraHeaders* p);
/*! UpnpExtraHeaders_set_resp */
UPNPLIB_API int UpnpExtraHeaders_set_resp(UpnpExtraHeaders* p,
                                          const DOMString s);
/*! UpnpExtraHeaders_get_resp_cstr */
// EXPORT_SPEC const char *UpnpExtraHeaders_get_resp_cstr(
//        const UpnpExtraHeaders *p);

#endif /* UPNPLIB_UPNPEXTRAHEADERS_HPP */
