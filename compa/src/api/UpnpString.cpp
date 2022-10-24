// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-10-18

#include "compa/UpnpString.hpp"
#include "umock/string.hpp"
#include "umock/stdlib.hpp"

namespace compa {

/*!
 * \brief Internal implementation of the class UpnpString.
 *
 * \internal
 */
struct SUpnpString {
    /*! \brief Length of the string excluding terminating null byte ('\0'). */
    size_t m_length;
    /*! \brief Pointer to a dynamically allocated area that holds the NULL
     * terminated string. */
    char* m_string;
};

size_t UpnpString_get_Length(const UpnpString* p) {
    if (!p)
        return 0;
    return ((struct SUpnpString*)p)->m_length;
}

const char* UpnpString_get_String(const UpnpString* p) {
    if (!p)
        return nullptr;
    return ((struct SUpnpString*)p)->m_string;
}

int UpnpString_set_String(UpnpString* p, const char* s) {
    if (!p || !s)
        return 0;
    char* q = umock::string_h.strdup(s);
    if (!q)
        goto error_handler1;
    umock::stdlib_h.free(((struct SUpnpString*)p)->m_string);
    ((struct SUpnpString*)p)->m_length = strlen(q);
    ((struct SUpnpString*)p)->m_string = q;

error_handler1:
    return q != NULL;
}

int UpnpString_set_StringN(UpnpString* p, const char* s, size_t n) {
    if (!p || !s)
        return 0;
    char* q = umock::string_h.strndup(s, n);
    if (!q)
        goto error_handler1;
    umock::stdlib_h.free(((struct SUpnpString*)p)->m_string);
    ((struct SUpnpString*)p)->m_length = strlen(q);
    ((struct SUpnpString*)p)->m_string = q;

error_handler1:
    return q != NULL;
}

void UpnpString_clear(UpnpString* p) {
    if (!p)
        return;
    ((struct SUpnpString*)p)->m_length = (size_t)0;
    /* No need to realloc now, will do later when needed. */
    ((struct SUpnpString*)p)->m_string[0] = 0;
}

} // namespace compa
