#ifndef COMPA_UPNPFILEINFO_HPP
#define COMPA_UPNPFILEINFO_HPP
// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-10-30

/*!
 * \file
 *
 * \brief Header file for UpnpFileInfo methods.
 */
#include "upnplib/visibility.hpp"
#include "ixml/ixml.hpp"

#ifdef _WIN32
#include "UpnpInet.hpp"
#include <sys/types.h>
#else
#include <sys/socket.h>
#endif // _WIN32

#include "UpnpString.hpp"
#include "list.hpp"

/*!
 * UpnpFileInfo
 */

// Since the following struct is completely invisible outside of pupnp (because
// of some template macro magic) I have duplicated it for testing here. The
// original is located in UpnpFileInfo.cpp. Possible differences of the copies
// in the future should be detected by the tests. --Ingo
struct s_UpnpFileInfo {
    off_t m_FileLength;
    time_t m_LastModified;
    int m_IsDirectory;
    int m_IsReadable;
    DOMString m_ContentType;
    UpnpListHead m_ExtraHeadersList;
    struct sockaddr_storage m_CtrlPtIPAddr;
    UpnpString* m_Os;
};

typedef struct s_UpnpFileInfo UpnpFileInfo;

namespace compa {

#include "list.hpp"

/*! UpnpFileInfo_get_FileLength */
UPNPLIB_API off_t UpnpFileInfo_get_FileLength(const UpnpFileInfo* p);
/*! UpnpFileInfo_set_FileLength */
UPNPLIB_API int UpnpFileInfo_set_FileLength(UpnpFileInfo* p, off_t n);
/*! UpnpFileInfo_get_LastModified */
UPNPLIB_API time_t UpnpFileInfo_get_LastModified(const UpnpFileInfo* p);
/*! UpnpFileInfo_set_LastModified */
UPNPLIB_API int UpnpFileInfo_set_LastModified(UpnpFileInfo* p, time_t n);
/*! UpnpFileInfo_get_IsDirectory */
UPNPLIB_API int UpnpFileInfo_get_IsDirectory(const UpnpFileInfo* p);
/*! UpnpFileInfo_set_IsDirectory */
UPNPLIB_API int UpnpFileInfo_set_IsDirectory(UpnpFileInfo* p, int n);
/*! UpnpFileInfo_get_IsReadable */
UPNPLIB_API int UpnpFileInfo_get_IsReadable(const UpnpFileInfo* p);
/*! UpnpFileInfo_set_IsReadable */
UPNPLIB_API int UpnpFileInfo_set_IsReadable(UpnpFileInfo* p, int n);
/*! UpnpFileInfo_get_ContentType */
UPNPLIB_API const DOMString UpnpFileInfo_get_ContentType(const UpnpFileInfo* p);
/*! UpnpFileInfo_set_ContentType */
UPNPLIB_API int UpnpFileInfo_set_ContentType(UpnpFileInfo* p,
                                             const DOMString s);
/*! UpnpFileInfo_get_ContentType_cstr */
UPNPLIB_API const char*
UpnpFileInfo_get_ContentType_cstr(const UpnpFileInfo* p);
/*! UpnpFileInfo_get_ExtraHeadersList */
UPNPLIB_API const UpnpListHead*
UpnpFileInfo_get_ExtraHeadersList(const UpnpFileInfo* p);
/*! UpnpFileInfo_set_ExtraHeadersList */
UPNPLIB_API int UpnpFileInfo_set_ExtraHeadersList(UpnpFileInfo* p,
                                                  const UpnpListHead* q);
/*! UpnpFileInfo_add_to_list_ExtraHeadersList */
UPNPLIB_API void UpnpFileInfo_add_to_list_ExtraHeadersList(UpnpFileInfo* p,
                                                           UpnpListHead* head);
/*! UpnpFileInfo_get_CtrlPtIPAddr */
UPNPLIB_API const struct sockaddr_storage*
UpnpFileInfo_get_CtrlPtIPAddr(const UpnpFileInfo* p);
/*! UpnpFileInfo_set_CtrlPtIPAddr */
UPNPLIB_API int
UpnpFileInfo_set_CtrlPtIPAddr(UpnpFileInfo* p,
                              const struct sockaddr_storage* buf);
/*! UpnpFileInfo_clear_CtrlPtIPAddr */
UPNPLIB_API void UpnpFileInfo_clear_CtrlPtIPAddr(UpnpFileInfo* p);
/*! UpnpFileInfo_get_Os */
UPNPLIB_API const UpnpString* UpnpFileInfo_get_Os(const UpnpFileInfo* p);
/*! UpnpFileInfo_set_Os */
UPNPLIB_API int UpnpFileInfo_set_Os(UpnpFileInfo* p, const UpnpString* s);
/*! UpnpFileInfo_get_Os_Length */
UPNPLIB_API size_t UpnpFileInfo_get_Os_Length(const UpnpFileInfo* p);
/*! UpnpFileInfo_get_Os_cstr */
UPNPLIB_API const char* UpnpFileInfo_get_Os_cstr(const UpnpFileInfo* p);
/*! UpnpFileInfo_strcpy_Os */
UPNPLIB_API int UpnpFileInfo_strcpy_Os(UpnpFileInfo* p, const char* s);
/*! UpnpFileInfo_strncpy_Os */
UPNPLIB_API int UpnpFileInfo_strncpy_Os(UpnpFileInfo* p, const char* s,
                                        size_t n);
/*! UpnpFileInfo_clear_Os */
UPNPLIB_API void UpnpFileInfo_clear_Os(UpnpFileInfo* p);

} // namespace compa

#endif /* COMPA_UPNPFILEINFO_HPP */
