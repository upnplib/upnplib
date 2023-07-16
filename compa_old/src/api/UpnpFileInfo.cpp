// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-04-27
// Also Copyright by other contributor as noted below.

/*!
 * \file
 *
 * \brief Source file for UpnpFileInfo methods.
 * \author Marcelo Roberto Jimenez
 */
#include <compa/UpnpFileInfo.hpp>
#include <upnplib/port.hpp>
#include <cstring> // for memset

namespace compa {

UpnpFileInfo* UpnpFileInfo_new() {
    struct UpnpFileInfo* p =
        (UpnpFileInfo*)calloc(1, sizeof(struct UpnpFileInfo));

    if (!p)
        return 0;

    /*p->m_FileLength = 0;*/
    /*p->m_LastModified = 0;*/
    /*p->m_IsDirectory = 0;*/
    /*p->m_IsReadable = 0;*/
    /*p->m_ContentType = 0;*/
    UpnpListInit(&p->m_ExtraHeadersList);
    /* memset(&p->m_CtrlPtIPAddr, 0, sizeof (struct sockaddr_storage)); */
    p->m_Os = UpnpString_new();

    return (UpnpFileInfo*)p;
}

void UpnpFileInfo_delete(UpnpFileInfo* q) {
    struct UpnpFileInfo* p = (struct UpnpFileInfo*)q;

    if (!p)
        return;

    UpnpString_delete(p->m_Os);
    p->m_Os = 0;
    memset(&p->m_CtrlPtIPAddr, 0, sizeof(struct sockaddr_storage));
    UpnpListInit(&p->m_ExtraHeadersList);
    ixmlFreeDOMString(p->m_ContentType);
    p->m_ContentType = 0;
    p->m_IsReadable = 0;
    p->m_IsDirectory = 0;
    p->m_LastModified = 0;
    p->m_FileLength = 0;

    free(p);
}

int UpnpFileInfo_assign(UpnpFileInfo* p, const UpnpFileInfo* q) {
    int ok = 1;

    if (p != q) {
        ok = ok &&
             UpnpFileInfo_set_FileLength(p, UpnpFileInfo_get_FileLength(q));
        ok = ok &&
             UpnpFileInfo_set_LastModified(p, UpnpFileInfo_get_LastModified(q));
        ok = ok &&
             UpnpFileInfo_set_IsDirectory(p, UpnpFileInfo_get_IsDirectory(q));
        ok = ok &&
             UpnpFileInfo_set_IsReadable(p, UpnpFileInfo_get_IsReadable(q));
        ok = ok &&
             UpnpFileInfo_set_ContentType(p, UpnpFileInfo_get_ContentType(q));
        ok = ok && UpnpFileInfo_set_ExtraHeadersList(
                       p, UpnpFileInfo_get_ExtraHeadersList(q));
        ok = ok &&
             UpnpFileInfo_set_CtrlPtIPAddr(p, UpnpFileInfo_get_CtrlPtIPAddr(q));
        ok = ok && UpnpFileInfo_set_Os(p, UpnpFileInfo_get_Os(q));
    }

    return ok;
}

UpnpFileInfo* UpnpFileInfo_dup(const UpnpFileInfo* q) {
    UpnpFileInfo* p = UpnpFileInfo_new();

    if (!p)
        return 0;

    UpnpFileInfo_assign(p, q);

    return p;
}

off_t UpnpFileInfo_get_FileLength(const UpnpFileInfo* p) {
    if (!p)
        return 0;
    return p->m_FileLength;
}

int UpnpFileInfo_set_FileLength(UpnpFileInfo* p, off_t n) {
    if (!p)
        return 0;

    p->m_FileLength = n;

    return 1;
}

time_t UpnpFileInfo_get_LastModified(const UpnpFileInfo* p) {
    if (!p)
        return 0;

    return p->m_LastModified;
}

int UpnpFileInfo_set_LastModified(UpnpFileInfo* p, time_t n) {
    if (!p)
        return 0;

    p->m_LastModified = n;

    return 1;
}

int UpnpFileInfo_get_IsDirectory(const UpnpFileInfo* p) {
    if (!p)
        return 0;

    return p->m_IsDirectory;
}

int UpnpFileInfo_set_IsDirectory(UpnpFileInfo* p, int n) {
    if (!p)
        return 0;

    p->m_IsDirectory = n;

    return 1;
}

int UpnpFileInfo_get_IsReadable(const UpnpFileInfo* p) {
    if (!p)
        return 0;

    return p->m_IsReadable;
}

int UpnpFileInfo_set_IsReadable(UpnpFileInfo* p, int n) {
    if (!p)
        return 0;

    p->m_IsReadable = n;

    return 1;
}

const DOMString UpnpFileInfo_get_ContentType(const UpnpFileInfo* p) {
    if (!p)
        return nullptr;
    return p->m_ContentType;
}

int UpnpFileInfo_set_ContentType(UpnpFileInfo* p, const DOMString s) {
    DOMString q = ixmlCloneDOMString(s);
    if (!p || !q)
        return 0;
    ixmlFreeDOMString(p->m_ContentType);
    p->m_ContentType = q;

    return 1;
}

const char* UpnpFileInfo_get_ContentType_cstr(const UpnpFileInfo* p) {
    return (const char*)UpnpFileInfo_get_ContentType(p);
}

const UpnpListHead* UpnpFileInfo_get_ExtraHeadersList(const UpnpFileInfo* p) {
    return &p->m_ExtraHeadersList;
}

int UpnpFileInfo_set_ExtraHeadersList(UpnpFileInfo* p, const UpnpListHead* q) {
    if (!p || !q)
        return 0;

    p->m_ExtraHeadersList = *q;

    return 1;
}

void UpnpFileInfo_add_to_list_ExtraHeadersList(UpnpFileInfo* p,
                                               struct UpnpListHead* head) {
    if (!p || !head)
        return;
    UpnpListHead* list = &p->m_ExtraHeadersList;
    UpnpListInsert(list, UpnpListEnd(list), head);
}

const sockaddr_storage* UpnpFileInfo_get_CtrlPtIPAddr(const UpnpFileInfo* p) {
    TRACE("Executing ::compa::UpnpFileInfo_get_CtrlPtIPAddr()")
    if (!p)
        return nullptr;

    return &p->m_CtrlPtIPAddr;
}

int UpnpFileInfo_set_CtrlPtIPAddr(UpnpFileInfo* p,
                                  const sockaddr_storage* buf) {
    if (!p || !buf)
        return 0;

    p->m_CtrlPtIPAddr = *buf;

    return 1;
}

void UpnpFileInfo_clear_CtrlPtIPAddr(UpnpFileInfo* p) {
    if (!p)
        return;

    memset(&p->m_CtrlPtIPAddr, 0, sizeof(sockaddr_storage));
}

const UpnpString* UpnpFileInfo_get_Os(const UpnpFileInfo* p) {
    if (!p)
        return nullptr;

    return p->m_Os;
}

int UpnpFileInfo_set_Os(UpnpFileInfo* p, const UpnpString* s) {
    if (!p || !s)
        return 0;

    const char* q = UpnpString_get_String(s);

    return UpnpString_set_String(p->m_Os, q);
}

size_t UpnpFileInfo_get_Os_Length(const UpnpFileInfo* p) {
    return UpnpString_get_Length(UpnpFileInfo_get_Os(p));
}

const char* UpnpFileInfo_get_Os_cstr(const UpnpFileInfo* p) {
    return UpnpString_get_String(UpnpFileInfo_get_Os(p));
}

int UpnpFileInfo_strcpy_Os(UpnpFileInfo* p, const char* s) {
    if (!p)
        return 0;

    return UpnpString_set_String(p->m_Os, s);
}

int UpnpFileInfo_strncpy_Os(UpnpFileInfo* p, const char* s, size_t n) {
    if (!p)
        return 0;

    return UpnpString_set_StringN(p->m_Os, s, n);
}

void UpnpFileInfo_clear_Os(UpnpFileInfo* p) {
    if (!p)
        return;

    UpnpString_clear(p->m_Os);
}

} // namespace compa
