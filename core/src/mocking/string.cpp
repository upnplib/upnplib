// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-10-07

#include "upnplib/mocking/string.hpp"

namespace upnplib {
namespace mocking {

char* StringReal::strdup(const char* s) { return ::strdup(s); }
char* StringReal::strndup(const char* s, size_t n) { return ::strndup(s, n); }

// This constructor is used to inject the pointer to the real function.
String::String(StringReal* a_ptr_realObj) {
    m_ptr_workerObj = (StringInterface*)a_ptr_realObj;
}

// This constructor is used to inject the pointer to the mocking function.
String::String(StringInterface* a_ptr_mockObj) {
    m_ptr_oldObj = m_ptr_workerObj;
    m_ptr_workerObj = a_ptr_mockObj;
}

// The destructor is ussed to restore the old pointer.
String::~String() { m_ptr_workerObj = m_ptr_oldObj; }

// Methods
char* String::strdup(const char* s) { return m_ptr_workerObj->strdup(s); }
char* String::strndup(const char* s, size_t n) {
    return m_ptr_workerObj->strndup(s, n);
}

// On program start create an object and inject pointer to the real functions.
// This will exist until program end.
StringReal string_realObj;
String string_h(&string_realObj);

} // namespace mocking
} // namespace upnplib
