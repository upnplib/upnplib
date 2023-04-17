#ifndef UMOCK_SYSINFO_MOCK_HPP
#define UMOCK_SYSINFO_MOCK_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-04-17

#include <umock/sysinfo.hpp>
#include <gmock/gmock.h>

namespace umock {

class SysinfoMock : public umock::SysinfoInterface {
  public:
    virtual ~SysinfoMock() override = default;
    MOCK_METHOD(time_t, time, (time_t * tloc), (override));
#ifndef _WIN32
    MOCK_METHOD(int, uname, (utsname * buf), (override));
#endif
};

} // namespace umock

#endif // UMOCK_SYSINFO_MOCK_HPP
