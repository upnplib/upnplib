#ifndef UMOCK_STRINGH_MOCK_HPP
#define UMOCK_STRINGH_MOCK_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-09-15

#include <umock/stringh.hpp>
#include <gmock/gmock.h>

namespace umock {

class UPNPLIB_API StringhMock : public umock::StringhInterface {
  public:
    StringhMock();
    virtual ~StringhMock() override;
#ifdef _MSC_VER
#pragma warning(push)
// This can be ignored for classes from the C++ STL (best if it is private).
#pragma warning(disable : 4251)
#endif
    MOCK_METHOD(char*, strerror, (int errnum), (override));
    MOCK_METHOD(char*, strdup, (const char* s), (override));
    MOCK_METHOD(char*, strndup, (const char* s, size_t n), (override));
#ifdef _MSC_VER
#pragma warning(pop)
#endif
};

} // namespace umock

#endif // UMOCK_STRINGH_MOCK_HPP
