#ifndef UMOCK_STDIO_MOCK_HPP
#define UMOCK_STDIO_MOCK_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-09-09

#include <umock/stdio.hpp>
#include <gmock/gmock.h>

namespace umock {

class StdioMock : public umock::StdioInterface {
  public:
    virtual ~StdioMock() override {}

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
};
// clang-format on

} // namespace umock

#endif // UMOCK_STDIO_MOCK_HPP
