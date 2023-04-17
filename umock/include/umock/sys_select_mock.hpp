#ifndef UMOCK_SYS_SELECT_MOCK_HPP
#define UMOCK_SYS_SELECT_MOCK_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-04-17

#include <umock/sys_select.hpp>
#include <gmock/gmock.h>

namespace umock {

class Sys_selectMock : public umock::Sys_selectInterface {
  public:
    virtual ~Sys_selectMock() override = default;
    MOCK_METHOD(int, select,
                (SOCKET nfds, fd_set* readfds, fd_set* writefds,
                 fd_set* exceptfds, struct timeval* timeout),
                (override));
};

} // namespace umock

#endif // UMOCK_SYS_SELECT_MOCK_HPP
