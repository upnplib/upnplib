#ifndef UMOCK_STDIO_MOCK_HPP
#define UMOCK_STDIO_MOCK_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-26

#include <umock/stdio.hpp>
#include <upnplib/port.hpp>
#include <gmock/gmock.h>

namespace umock {

class UPNPLIB_API StdioMock : public umock::StdioInterface {
  public:
    StdioMock();
    virtual ~StdioMock() override;
    DISABLE_MSVC_WARN_4251
// clang-format off
#ifdef _WIN32
    // Secure function only on MS Windows
    MOCK_METHOD(errno_t, fopen_s, (FILE * *pFile, const char* pathname, const char* mode), (override));
#endif
    MOCK_METHOD(FILE*, fopen, (const char* pathname, const char* mode), (override));
    MOCK_METHOD(int, fclose, (FILE * stream), (override));
    MOCK_METHOD(size_t, fread, (void* ptr, size_t size, size_t nmemb, FILE* stream), (override));
    MOCK_METHOD(size_t, fwrite, (const void* ptr, size_t size, size_t nmemb, FILE* stream), (override));
    MOCK_METHOD(int, fflush, (FILE * stream), (override));
    ENABLE_MSVC_WARN
};
// clang-format on

} // namespace umock

#endif // UMOCK_STDIO_MOCK_HPP
