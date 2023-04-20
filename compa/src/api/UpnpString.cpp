// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-04-27

#include <compa/UpnpString.hpp>
#include <upnplib/port.hpp>
#include <umock/stdlib.hpp>
#include <umock/stringh.hpp>

namespace compa {

UpnpString* UpnpString_new() {
    /* All bytes are zero, and so is the length of the string. */
    struct UpnpString* p = (UpnpString*)umock::stdlib_h.calloc(
        (size_t)1, sizeof(struct UpnpString));
    if (p == NULL) {
        goto error_handler1;
    }
#if 0
	p->m_length = 0;
#endif

    /* This byte is zero, calloc does initialize it. */
    p->m_string = (char*)umock::stdlib_h.calloc((size_t)1, (size_t)1);
    if (p->m_string == NULL) {
        goto error_handler2;
    }

    return (UpnpString*)p;

    /*free(p->m_string); */
error_handler2:
    umock::stdlib_h.free(p);
error_handler1:
    return NULL;
}

void UpnpString_delete(UpnpString* p) {
    struct UpnpString* q = (struct UpnpString*)p;

    if (!q)
        return;

    q->m_length = (size_t)0;

    umock::stdlib_h.free(q->m_string);
    q->m_string = NULL;

    umock::stdlib_h.free(p);
}

UpnpString* UpnpString_dup(const UpnpString* p) {
    struct UpnpString* q = (UpnpString*)umock::stdlib_h.calloc(
        (size_t)1, sizeof(struct UpnpString));
    if (q == NULL) {
        goto error_handler1;
    }
    q->m_length = ((struct UpnpString*)p)->m_length;
    q->m_string = umock::string_h.strdup(((struct UpnpString*)p)->m_string);
    if (q->m_string == NULL) {
        goto error_handler2;
    }

    return (UpnpString*)q;

    /*free(q->m_string); */
error_handler2:
    umock::stdlib_h.free(q);
error_handler1:
    return NULL;
}

[[maybe_unused]] void UpnpString_assign(UpnpString* p, const UpnpString* q) {
    if (p != q) {
        UpnpString_set_String(p, UpnpString_get_String(q));
    }
}

size_t UpnpString_get_Length(const UpnpString* p) {
    if (!p)
        return 0;
    return ((struct UpnpString*)p)->m_length;
}

void UpnpString_set_Length(UpnpString* p, size_t n) {
    if (((struct UpnpString*)p)->m_length > n) {
        ((struct UpnpString*)p)->m_length = n;
        /* No need to realloc now, will do later when needed. */
        ((struct UpnpString*)p)->m_string[n] = 0;
    }
}

const char* UpnpString_get_String(const UpnpString* p) {
    if (!p)
        return nullptr;
    return ((struct UpnpString*)p)->m_string;
}

int UpnpString_set_String(UpnpString* p, const char* s) {
    if (!p || !s)
        return 0;
    char* q = umock::string_h.strdup(s);
    if (!q)
        goto error_handler1;
    umock::stdlib_h.free(((struct UpnpString*)p)->m_string);
    ((struct UpnpString*)p)->m_length = strlen(q);
    ((struct UpnpString*)p)->m_string = q;

error_handler1:
    return q != NULL;
}

int UpnpString_set_StringN(UpnpString* p, const char* s, size_t n) {
    if (!p || !s)
        return 0;
    char* q = umock::string_h.strndup(s, n);
    if (!q)
        goto error_handler1;
    umock::stdlib_h.free(((struct UpnpString*)p)->m_string);
    ((struct UpnpString*)p)->m_length = strlen(q);
    ((struct UpnpString*)p)->m_string = q;

error_handler1:
    return q != NULL;
}

void UpnpString_clear(UpnpString* p) {
    if (!p)
        return;
    ((struct UpnpString*)p)->m_length = (size_t)0;
    /* No need to realloc now, will do later when needed. */
    ((struct UpnpString*)p)->m_string[0] = 0;
}

int UpnpString_cmp(UpnpString* p, UpnpString* q) {
    const char* cp = UpnpString_get_String(p);
    const char* cq = UpnpString_get_String(q);

    return strcmp(cp, cq);
}

int UpnpString_casecmp(UpnpString* p, UpnpString* q) {
    const char* cp = UpnpString_get_String(p);
    const char* cq = UpnpString_get_String(q);

    return strcasecmp(cp, cq);
}

} // namespace compa
