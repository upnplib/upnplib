#ifndef UMOCK_UNISTD_MOCK_HPP
#define UMOCK_UNISTD_MOCK_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-26

#include <umock/unistd.hpp>
#include <upnplib/port.hpp>
#include <gmock/gmock.h>

namespace umock {

class UPNPLIB_API UnistdMock : public UnistdInterface {
  public:
    UnistdMock();
    virtual ~UnistdMock() override;
    DISABLE_MSVC_WARN_4251
    MOCK_METHOD(int, CLOSE_SOCKET_P, (SOCKET fd), (override));
    ENABLE_MSVC_WARN
};

} // namespace umock

#endif // UMOCK_UNISTD_MOCK_HPP
