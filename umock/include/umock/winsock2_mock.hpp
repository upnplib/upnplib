#ifndef UMOCK_WINSOCK2_MOCK_HPP
#define UMOCK_WINSOCK2_MOCK_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-08-05

#include <umock/winsock2.hpp>
#include <gmock/gmock.h>

namespace umock {

class Winsock2Mock : public umock::Winsock2Interface {
  public:
    virtual ~Winsock2Mock() override = default;
    MOCK_METHOD(int, WSAGetLastError, (), (override));
};

} // namespace umock

#endif // UMOCK_WINSOCK2_MOCK_HPP
