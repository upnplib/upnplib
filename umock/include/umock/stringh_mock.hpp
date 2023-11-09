#ifndef UMOCK_STRINGH_MOCK_HPP
#define UMOCK_STRINGH_MOCK_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-11-16

#include <umock/stringh.hpp>
#include <gmock/gmock.h>
#include <upnplib/port.hpp>

namespace umock {

class UPNPLIB_API StringhMock : public umock::StringhInterface {
  public:
    StringhMock();
    virtual ~StringhMock() override;
    DISABLE_MSVC_WARN_4251
    MOCK_METHOD(char*, strerror, (int errnum), (override));
    MOCK_METHOD(char*, strdup, (const char* s), (override));
    MOCK_METHOD(char*, strndup, (const char* s, size_t n), (override));
    ENABLE_MSVC_WARN
};

} // namespace umock

#endif // UMOCK_STRINGH_MOCK_HPP
