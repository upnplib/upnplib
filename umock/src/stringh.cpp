// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-02-05

#include "umock/stringh.inc"
#include <cstdlib> // for malloc

//
namespace umock {

char* StringhReal::strerror(int errnum) { return ::strerror(errnum); }
char* StringhReal::strdup(const char* s) { return ::strdup(s); }
char* StringhReal::strndup(const char* s, size_t n) { return ::strndup(s, n); }

// This constructor is used to inject the pointer to the real function.
Stringh::Stringh(StringhReal* a_ptr_realObj) {
    m_ptr_workerObj = (StringhInterface*)a_ptr_realObj;
}

// This constructor is used to inject the pointer to the mocking function.
Stringh::Stringh(StringhInterface* a_ptr_mockObj) {
    m_ptr_oldObj = m_ptr_workerObj;
    m_ptr_workerObj = a_ptr_mockObj;
}

// The destructor is ussed to restore the old pointer.
Stringh::~Stringh() { m_ptr_workerObj = m_ptr_oldObj; }

// Methods
char* Stringh::strerror(int errnum) {
    return m_ptr_workerObj->strerror(errnum);
}
char* Stringh::strdup(const char* s) { return m_ptr_workerObj->strdup(s); }
char* Stringh::strndup(const char* s, size_t n) {
    return m_ptr_workerObj->strndup(s, n);
}

// On program start create an object and inject pointer to the real functions.
// This will exist until program end.
StringhReal stringh_realObj;
UPNPLIB_API Stringh string_h(&stringh_realObj);

} // namespace umock
