#ifndef UMOCK_TIMEH_MOCK_HPP
#define UMOCK_TIMEH_MOCK_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-02-05

#include "umock/timeh.hpp"

namespace umock {

class TimehMock : public umock::TimehInterface {
  public:
    virtual ~TimehMock() override = default;
    MOCK_METHOD(time_t, time, (time_t * tloc), (override));
};

} // namespace umock

#endif // UMOCK_TIMEH_MOCK_HPP
