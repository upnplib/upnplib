// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-09-25

#include "upnplib/mocking/stdio.hpp"

namespace upnplib {
namespace mocking {

#ifdef _WIN32
// Secure function only on MS Windows
errno_t StdioReal::fopen_s(FILE** pFile, const char* pathname,
                           const char* mode) {
    return ::fopen_s(pFile, pathname, mode);
}
#endif

FILE* StdioReal::fopen(const char* pathname, const char* mode) {
    return ::fopen(pathname, mode);
}

int StdioReal::fclose(FILE* stream) { return ::fclose(stream); }

int StdioReal::fflush(FILE* stream) { return ::fflush(stream); }

//
// This constructor is used to inject the pointer to the real function.
Stdio::Stdio(StdioReal* a_ptr_realObj) {
    m_ptr_workerObj = (StdioInterface*)a_ptr_realObj;
}

// This constructor is used to inject the pointer to the mocking function.
Stdio::Stdio(StdioInterface* a_ptr_mockObj) {
    m_ptr_oldObj = m_ptr_workerObj;
    m_ptr_workerObj = a_ptr_mockObj;
}

// The destructor is ussed to restore the old pointer.
Stdio::~Stdio() { m_ptr_workerObj = m_ptr_oldObj; }

#ifdef _WIN32
// Secure function only on MS Windows
errno_t Stdio::fopen_s(FILE** pFile, const char* pathname, const char* mode) {
    return m_ptr_workerObj->fopen_s(pFile, pathname, mode);
}
#endif

FILE* Stdio::fopen(const char* pathname, const char* mode) {
    return m_ptr_workerObj->fopen(pathname, mode);
}

int Stdio::fclose(FILE* stream) { return m_ptr_workerObj->fclose(stream); }

int Stdio::fflush(FILE* stream) { return m_ptr_workerObj->fflush(stream); }

// On program start create an object and inject pointer to the real functions.
// This will exist until program end.
StdioReal stdio_realObj;
Stdio stdio_h(&stdio_realObj);

} // namespace mocking
} // namespace upnplib
