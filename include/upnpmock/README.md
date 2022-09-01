# Mock system library calls in traditional C

C++ GoogleMock is mainly [working with Dependency Injection](https://google.github.io/googletest/gmock_for_dummies.html#a-case-for-mock-turtles):

> Fortunately, you learned about [Dependency Injection](http://en.wikipedia.org/wiki/Dependency_injection) and know the right thing to do: instead of having your application talk to the system API directly, wrap the API in an interface and code to that interface.

This is a typical C++ issue but cannot be done with traditional C. The idea is to use a base wrapper class for system functions and its methods just call the underlaying system functions, for example with the simple system function `char* strerror(int errnum);`

    class Bwrap_string_h {
      public:
        virtual ~Bwrap_string_h() {}

        virtual char* strerror(int errnum) {
            return ::strerror(errnum);
        }
    };

and call it in the production code with:

    ~$ Bwrap_string_h string_hObj{};
    ~$ msg = string_hObj.strerror(errno);

The only difference to a direct system call is that we have to prepend the object name, in this example `string_hObj`.

For mocking we need to replace the call to the system function with a call of the Mock macro. First we create the Mock as derived class:

    class CMock_string_h : Bwrap_string_h {
      public:
        MOCK_METHOD(char*, strerror, (int), (override));
    };

To be able to switch the call to the function from system call to mocked call we will use a pointer. It is initialized with pointing to the system call by default:

    ~$ Bwrap_string_h string_hObj{};
    ~$ Bwrap_string_h* string_h = &string_hObj;

    # and call it with
    ~$ char* msg = string_h->strerror(errno);

Now with instantiating the Mock object we will switch the pointer with the constructor/destructor following the RAII paradigm:

    class CMock_string_h : Bwrap_string_h {
        Bwrap_string_h* m_realptr;

      public:
        // Switch and save/restore the old pointer to the real function
        CMock_string_h() {
            m_realptr = string_h;
            string_h = this;
        }
        virtual ~Mock_string_h() override {
            string_h = m_realptr;
        }

        MOCK_METHOD(char*, strerror, (int), (override));
    };

Now we can instantiate a Mock object in a test suite and using it:

    CMock_string_h mocked_string_h;
    EXPECT_CALL(mocked_string_h, strerror(_))
        .WillOnce(Return("Mocked error message"));

To put it togther to a running system place the base class into a header file, the global base object variable and its pointer into a source file and the derived Mock class into a test suite. You must link the global variables with the program and you must #include the header file into every source file where you call a wrapped system function. The following example is only copied from my developement system but not tested so far but it should show what I mean.

The header file `string.hpp`

    #ifndef WRAP_STRING_H_HPP
    #define WRAP_STRING_H_HPP

    #include <string.h>

    class Bwrap_string_h {
        // Base class to call the system functions
      public:
        virtual ~Bwrap_string_h() {}

        virtual char* strerror(int errnum) { return ::strerror(errnum); }
        // Here you can add other functions from header <string.h>
    };

    // Global pointer to the current object (real or mocked), will be modified by
    // the constructor of the mock object.
    extern Bwrap_string_h* string_h;

    #endif // WRAP_STRING_H_HPP

The source file with the global variables `global.cpp`

    #include "string.hpp"

    ~$ Bwrap_string_h string_hObj{};
    ~$ Bwrap_string_h* string_h = &string_hObj;

Snippet from any source file that uses mockable system calls:

    #include "string.hpp"

    ~$ char* msg = string_h->strerror(errno);

Snippet from a test file `test_foo.cpp`

    #include "gmock/gmock.h"
    #include "string.hpp"

    using ::testing::_;
    using ::testing::Return;

    // Derived Class to mock the free system functions.
    class CMock_string_h : Bwrap_string_h {
        Bwrap_string_h* m_realptr;

      public:
        // Switch and save/restore the old pointer to the real function
        CMock_string_h() {
            m_realptr = string_h;
            string_h = this;
        }
        virtual ~Mock_string_h() override {
            string_h = m_realptr;
        }

        MOCK_METHOD(char*, strerror, (int), (override));
    };

    class StringhMockTestSuite : public ::testing::Test {
      protected:
        CMock_string_h mocked_string_h;
    };

    TEST_F(StringhMockTestSuite, foo_function_test)
    {
        EXPECT_CALL(mocked_string_h, strerror(_))
            .WillOnce(Return("Mocked error message"));
        EXPECT_EQ(function_with_mocked_system_calls, vallue);
        ....
    }

    int main(int argc, char** argv) {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }

In this upnplib project you find the
header files in `upnp/inc/upnpmock/`
global variables in `upnp/src/global.cpp`
test suites in `gtests/test_*`

<pre><sup>
// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  &#60;Ingo&#64;Hoeft-online.de&#62;
// Redistribution only with this Copyright remark. Last modified: 2022-08-31
</sup></sup>
