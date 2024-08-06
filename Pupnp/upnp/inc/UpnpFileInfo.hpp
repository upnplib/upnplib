#ifndef PUPNP_UPNPFILEINFO_HPP
#define PUPNP_UPNPFILEINFO_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-08-13
// Also Copyright by other contributor as noted below.
// Last compare with pupnp original source file on 2023-04-25, ver 1.14.15

/*!
 * \file
 *
 * \brief Header file for UpnpFileInfo methods.
 * \authors Marcelo Roberto Jimenez, Ingo Höft
 */
#include <stdlib.h> /* for size_t */

#include "UpnpGlobal.hpp" /* for UPNP_EXPORT_SPEC */
#ifdef _WIN32
#include "UpnpInet.hpp"
#else
#include <sys/socket.h>
#endif // _WIN32

#include "UpnpString.hpp"
#include "ixml.hpp"
#include "list.hpp"
#include <sys/types.h> // needed for off_t

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * UpnpFileInfo
 */
typedef struct s_UpnpFileInfo UpnpFileInfo;

/*! Constructor */
EXPORT_SPEC UpnpFileInfo* UpnpFileInfo_new();
/*! Destructor */
EXPORT_SPEC void UpnpFileInfo_delete(UpnpFileInfo* p);
/*! Copy Constructor */
EXPORT_SPEC UpnpFileInfo* UpnpFileInfo_dup(const UpnpFileInfo* p);
/*! Assignment operator */
EXPORT_SPEC int UpnpFileInfo_assign(UpnpFileInfo* p, const UpnpFileInfo* q);

/*! UpnpFileInfo_get_FileLength */
EXPORT_SPEC off_t UpnpFileInfo_get_FileLength(const UpnpFileInfo* p);
/*! UpnpFileInfo_set_FileLength */
EXPORT_SPEC int UpnpFileInfo_set_FileLength(UpnpFileInfo* p, off_t n);

/*! UpnpFileInfo_get_LastModified */
EXPORT_SPEC time_t UpnpFileInfo_get_LastModified(const UpnpFileInfo* p);
/*! UpnpFileInfo_set_LastModified */
EXPORT_SPEC int UpnpFileInfo_set_LastModified(UpnpFileInfo* p, time_t n);

/*! UpnpFileInfo_get_IsDirectory */
EXPORT_SPEC int UpnpFileInfo_get_IsDirectory(const UpnpFileInfo* p);
/*! UpnpFileInfo_set_IsDirectory */
EXPORT_SPEC int UpnpFileInfo_set_IsDirectory(UpnpFileInfo* p, int n);

/*! UpnpFileInfo_get_IsReadable */
EXPORT_SPEC int UpnpFileInfo_get_IsReadable(const UpnpFileInfo* p);
/*! UpnpFileInfo_set_IsReadable */
EXPORT_SPEC int UpnpFileInfo_set_IsReadable(UpnpFileInfo* p, int n);

/*! UpnpFileInfo_get_ContentType */
EXPORT_SPEC const DOMString UpnpFileInfo_get_ContentType(const UpnpFileInfo* p);
/*! UpnpFileInfo_set_ContentType */
EXPORT_SPEC int UpnpFileInfo_set_ContentType(UpnpFileInfo* p,
                                             const DOMString s);
/*! UpnpFileInfo_get_ContentType_cstr */
EXPORT_SPEC const char*
UpnpFileInfo_get_ContentType_cstr(const UpnpFileInfo* p);

/*! UpnpFileInfo_get_ExtraHeadersList */
EXPORT_SPEC const UpnpListHead*
UpnpFileInfo_get_ExtraHeadersList(const UpnpFileInfo* p);
/*! UpnpFileInfo_set_ExtraHeadersList */
EXPORT_SPEC int UpnpFileInfo_set_ExtraHeadersList(UpnpFileInfo* p,
                                                  const UpnpListHead* q);
/*! UpnpFileInfo_add_to_list_ExtraHeadersList */
EXPORT_SPEC void UpnpFileInfo_add_to_list_ExtraHeadersList(UpnpFileInfo* p,
                                                           UpnpListHead* head);

/*! UpnpFileInfo_get_CtrlPtIPAddr */
EXPORT_SPEC const struct sockaddr_storage*
UpnpFileInfo_get_CtrlPtIPAddr(const UpnpFileInfo* p);
/*! UpnpFileInfo_get_CtrlPtIPAddr */
EXPORT_SPEC int
UpnpFileInfo_set_CtrlPtIPAddr(UpnpFileInfo* p,
                              const struct sockaddr_storage* buf);
/*! UpnpFileInfo_get_CtrlPtIPAddr */
EXPORT_SPEC void UpnpFileInfo_clear_CtrlPtIPAddr(UpnpFileInfo* p);

/*! UpnpFileInfo_get_Os */
EXPORT_SPEC const UpnpString* UpnpFileInfo_get_Os(const UpnpFileInfo* p);
/*! UpnpFileInfo_set_Os */
EXPORT_SPEC int UpnpFileInfo_set_Os(UpnpFileInfo* p, const UpnpString* s);
/*! UpnpFileInfo_get_Os_Length */
EXPORT_SPEC size_t UpnpFileInfo_get_Os_Length(const UpnpFileInfo* p);
/*! UpnpFileInfo_get_Os_cstr */
EXPORT_SPEC const char* UpnpFileInfo_get_Os_cstr(const UpnpFileInfo* p);
/*! UpnpFileInfo_strcpy_Os */
EXPORT_SPEC int UpnpFileInfo_strcpy_Os(UpnpFileInfo* p, const char* s);
/*! UpnpFileInfo_strncpy_Os */
EXPORT_SPEC int UpnpFileInfo_strncpy_Os(UpnpFileInfo* p, const char* s,
                                        size_t n);
/*! UpnpFileInfo_clear_Os */
EXPORT_SPEC void UpnpFileInfo_clear_Os(UpnpFileInfo* p);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PUPNP_UPNPFILEINFO_HPP */
