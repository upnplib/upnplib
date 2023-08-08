#ifndef UMOCK_ARPA_INET_MOCK_HPP
#define UMOCK_ARPA_INET_MOCK_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-08-09

#include <umock/arpa_inet.hpp>
#include <gmock/gmock.h>

namespace umock {

class Arpa_inetMock : public umock::Arpa_inetInterface {
  public:
    virtual ~Arpa_inetMock() override = default;
    MOCK_METHOD(const char*, inet_ntop,
                (int af, const void* src, char* dst, socklen_t size),
                (override));
};

} // namespace umock

#endif // UMOCK_ARPA_INET_MOCK_HPP
