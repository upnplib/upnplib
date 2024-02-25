#ifndef COMPA_UPNPEXTRAHEADERS_HPP
#define COMPA_UPNPEXTRAHEADERS_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-02-27
/*!
 * \file
 * \brief Header file for UpnpExtraHeaders methods.
 * \authors Marcelo Roberto Jimenez, Ingo Höft
 */
// Don't export function symbols; only used library intern.

#include <UpnpString.hpp>
#include <ixml.hpp>
#include <list.hpp>

/*!
 * UpnpExtraHeaders
 */
typedef struct s_UpnpExtraHeaders UpnpExtraHeaders;

/*! Constructor */
UpnpExtraHeaders* UpnpExtraHeaders_new();
/*! Destructor */
void UpnpExtraHeaders_delete(UpnpExtraHeaders* p);
/*! Copy Constructor */
UpnpExtraHeaders* UpnpExtraHeaders_dup(const UpnpExtraHeaders* p);
/*! Assignment operator */
int UpnpExtraHeaders_assign(UpnpExtraHeaders* p, const UpnpExtraHeaders* q);

/*! UpnpExtraHeaders_get_node */
const UpnpListHead* UpnpExtraHeaders_get_node(const UpnpExtraHeaders* p);
/*! UpnpExtraHeaders_set_node */
int UpnpExtraHeaders_set_node(UpnpExtraHeaders* p, const UpnpListHead* q);
/*! UpnpExtraHeaders_add_to_list_node */
void UpnpExtraHeaders_add_to_list_node(UpnpExtraHeaders* p, UpnpListHead* head);

/*! UpnpExtraHeaders_get_name */
const UpnpString* UpnpExtraHeaders_get_name(const UpnpExtraHeaders* p);
/*! UpnpExtraHeaders_set_name */
int UpnpExtraHeaders_set_name(UpnpExtraHeaders* p, const UpnpString* s);
/*! UpnpExtraHeaders_get_name_Length */
size_t UpnpExtraHeaders_get_name_Length(const UpnpExtraHeaders* p);
/*! UpnpExtraHeaders_get_name_cstr */
const char* UpnpExtraHeaders_get_name_cstr(const UpnpExtraHeaders* p);
/*! UpnpExtraHeaders_strcpy_name */
int UpnpExtraHeaders_strcpy_name(UpnpExtraHeaders* p, const char* s);
/*! UpnpExtraHeaders_strncpy_name */
int UpnpExtraHeaders_strncpy_name(UpnpExtraHeaders* p, const char* s, size_t n);
/*! UpnpExtraHeaders_clear_name */
void UpnpExtraHeaders_clear_name(UpnpExtraHeaders* p);

/*! UpnpExtraHeaders_get_value */
const UpnpString* UpnpExtraHeaders_get_value(const UpnpExtraHeaders* p);
/*! UpnpExtraHeaders_set_value */
int UpnpExtraHeaders_set_value(UpnpExtraHeaders* p, const UpnpString* s);
/*! UpnpExtraHeaders_get_value_Length */
size_t UpnpExtraHeaders_get_value_Length(const UpnpExtraHeaders* p);
/*! UpnpExtraHeaders_get_value_cstr */
const char* UpnpExtraHeaders_get_value_cstr(const UpnpExtraHeaders* p);
/*! UpnpExtraHeaders_strcpy_value */
int UpnpExtraHeaders_strcpy_value(UpnpExtraHeaders* p, const char* s);
/*! UpnpExtraHeaders_strncpy_value */
int UpnpExtraHeaders_strncpy_value(UpnpExtraHeaders* p, const char* s,
                                   size_t n);
/*! UpnpExtraHeaders_clear_value */
void UpnpExtraHeaders_clear_value(UpnpExtraHeaders* p);

/*! UpnpExtraHeaders_get_resp */
const DOMString UpnpExtraHeaders_get_resp(const UpnpExtraHeaders* p);
/*! UpnpExtraHeaders_set_resp */
int UpnpExtraHeaders_set_resp(UpnpExtraHeaders* p, const DOMString s);
/*! UpnpExtraHeaders_get_resp_cstr */
const char* UpnpExtraHeaders_get_resp_cstr(const UpnpExtraHeaders* p);

#endif /* COMPA_UPNPEXTRAHEADERS_HPP */
