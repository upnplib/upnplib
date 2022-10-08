// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-10-08

#include "compa/UpnpString.hpp"
#include <iostream> // DEBUG!

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
    std::cout << "DEBUG! Tracepoint current\n";
    return ((struct SUpnpString*)p)->m_length;
}

} // namespace compa
