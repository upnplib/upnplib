#ifndef UMOCK_UNISTD_MOCK_HPP
#define UMOCK_UNISTD_MOCK_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-08-09

#include <umock/unistd.hpp>
#include <gmock/gmock.h>

namespace umock {

class UnistdMock : public UnistdInterface {
  public:
    virtual ~UnistdMock() override = default;
    MOCK_METHOD(int, CLOSE_SOCKET_P, (SOCKET fd), (override));
};

} // namespace umock

#endif // UMOCK_UNISTD_MOCK_HPP