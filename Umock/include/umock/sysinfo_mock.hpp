#ifndef UMOCK_SYSINFO_MOCK_HPP
#define UMOCK_SYSINFO_MOCK_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-26

#include <umock/sysinfo.hpp>
#include <upnplib/port.hpp>
#include <gmock/gmock.h>

namespace umock {

class UPNPLIB_API SysinfoMock : public umock::SysinfoInterface {
  public:
    SysinfoMock();
    virtual ~SysinfoMock() override;
    DISABLE_MSVC_WARN_4251
    MOCK_METHOD(time_t, time, (time_t * tloc), (override));
#ifndef _WIN32
    MOCK_METHOD(int, uname, (utsname * buf), (override));
#endif
    ENABLE_MSVC_WARN
};

} // namespace umock

#endif // UMOCK_SYSINFO_MOCK_HPP
