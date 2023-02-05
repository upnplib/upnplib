// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-12-05

// There is no include guard '#ifndef ...' because this file shouldn't be
// included more than two times as given.

#include "upnplib/visibility.hpp"
#include <ctime>

namespace umock {

class TimehInterface {
  public:
    virtual ~TimehInterface() = default;
    virtual time_t time(time_t* tloc) = 0;
};

//
// This is the wrapper class (worker) for the real (library?) function
// -------------------------------------------------------------------
class TimehReal : public TimehInterface {
  public:
    virtual ~TimehReal() override = default;
    time_t time(time_t* tloc) override;
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
/* Example:
    TimehReal timeh_realObj;       // already done
    Timeh time_h(&timeh_realObj); // already done
    { // Other scope, e.g. within a gtest
        class TimehMock : public TimehInterface { ...; MOCK_METHOD(...) };
        TimehMock timeh_mockObj;
        Timeh timeh_injectObj(&string_mockObj); // obj. name doesn't matter
        EXPECT_CALL(timeh_mockObj, ...);
    } // End scope, mock objects are destructed, worker restored to default.
*/ //------------------------------------------------------------------------
class UPNPLIB_API Timeh {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Timeh(TimehReal* a_ptr_realObj);

    // This constructor is used to inject the pointer to the mocking function.
    Timeh(TimehInterface* a_ptr_mockObj);

    // The destructor is ussed to restore the old pointer.
    virtual ~Timeh();

    // Methods
    virtual time_t time(time_t* tloc);

  private:
    // Next variable must be static. Please note that a static member variable
    // belongs to the class, but not to the instantiated object. This is
    // important here for mocking because the pointer is also valid on all
    // objects of this class. With inline we do not need an extra definition
    // line outside the class. I also make the symbol hidden so the variable
    // cannot be accessed globaly with Timeh::m_ptr_workerObj. --Ingo
    UPNPLIB_LOCAL static inline TimehInterface* m_ptr_workerObj;
    TimehInterface* m_ptr_oldObj{};
};

} // namespace umock
// vim: syntax=cpp
