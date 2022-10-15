// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-10-13

#include "UpnpString.hpp"

#ifdef UPNPLIB_WITH_NATIVE_PUPNP
#define NS
#else
#define NS compa
#include "compa/UpnpString.hpp"
#endif

#include "upnplib/gtest.hpp"
#include "upnplib/mocking/stdlib.hpp"
#include "upnplib/mocking/string.hpp"
#include "gmock/gmock.h"

using ::testing::_;
using ::testing::Eq;
using ::testing::ExitedWithCode;
using ::testing::Return;

namespace mock = upnplib::mocking;

//
// Since the following struct is completely invisible outside of pupnp (because
// of some template macro magic) I have duplicated it for testing here. The
// original is located in UpnpString.cpp. Possible differences of the two
// copies in the future should be detected by the tests. --Ingo
struct s_UpnpString {
    /*! \brief Length of the string excluding terminating null byte ('\0'). */
    size_t m_length;
    /*! \brief Pointer to a dynamically allocated area that holds the NULL
     * terminated string. */
    char* m_string;
};

namespace compa {
bool old_code{false}; // Managed in upnplib_gtest_main.inc
bool github_actions = std::getenv("GITHUB_ACTIONS");

//
// Mocked system calls
//--------------------
class StdlibMock : public mock::StdlibInterface {
  public:
    virtual ~StdlibMock() override {}
    MOCK_METHOD(void*, malloc, (size_t size), (override));
    MOCK_METHOD(void, free, (void* ptr), (override));
    MOCK_METHOD(void*, calloc, (size_t nmemb, size_t size), (override));
};

class StringMock : public mock::StringInterface {
  public:
    virtual ~StringMock() override {}
    MOCK_METHOD(char*, strdup, (const char* s), (override));
    MOCK_METHOD(char*, strndup, (const char* s, size_t n), (override));
};

// testsuite with fixtures
//------------------------
class UpnpStringMockTestSuite : public ::testing::Test {
  protected:
    StdlibMock mock_stdlibObj;
};

TEST_F(UpnpStringMockTestSuite, create_new_upnp_string) {
    // provide a structure of a UpnpString
    char mstring[]{""};
    UpnpString upnpstr{};
    UpnpString* p = &upnpstr;
    UpnpString* str{};

    mock::Stdlib stdlib_injectObj(&this->mock_stdlibObj);
    EXPECT_CALL(this->mock_stdlibObj, calloc(_, _))
        .WillOnce(Return(p))
        .WillOnce(Return(mstring));

    // Test Unit
    str = UpnpString_new();

    // str should point to the upnpstr structure
    EXPECT_EQ(str, &upnpstr);
    EXPECT_EQ(str->m_length, 0);
    // and its member 'm_string' should point to an empty string (mstring) that
    // is also allocated on the heap (if not mocked like here).
    EXPECT_EQ(str->m_string, mstring);
    EXPECT_EQ(upnpstr.m_string, mstring);
    EXPECT_STREQ(str->m_string, "");

    // test edge conditions
    EXPECT_CALL(this->mock_stdlibObj, calloc(_, _)).WillOnce(Return(nullptr));

    // Test Unit
    str = UpnpString_new();
    EXPECT_EQ(str, nullptr);

    EXPECT_CALL(this->mock_stdlibObj, calloc(_, _))
        .WillOnce(Return(p))
        .WillOnce(Return(nullptr));
    EXPECT_CALL(this->mock_stdlibObj, free(_)).Times(1);

    // Test Unit
    str = UpnpString_new();
    EXPECT_EQ(str, nullptr);
}

TEST_F(UpnpStringMockTestSuite, delete_upnp_string) {
    // provide an UpnpString
    char mstring[] = "hello world";
    UpnpString upnpstr = {11, mstring};
    UpnpString* p = &upnpstr;

    // Test Unit, check edge condition
    UpnpString_delete(nullptr);

    EXPECT_EQ(upnpstr.m_length, 11);
    EXPECT_STREQ(upnpstr.m_string, "hello world");

    mock::Stdlib stdlib_injectObj(&mock_stdlibObj);
    EXPECT_CALL(this->mock_stdlibObj, free(_)).Times(2);

    // Test Unit
    UpnpString_delete(p);

    EXPECT_EQ(upnpstr.m_length, 0);
    EXPECT_EQ(upnpstr.m_string, nullptr);
}

TEST_F(UpnpStringMockTestSuite, set_upnp_string) {
    // provide an empty UpnpString
    char mstring1[]{""};
    UpnpString upnpstr{0, mstring1};
    UpnpString* p = &upnpstr;

    char mstring2[]{"set string"}; // This string will set to the UpnpString.
    // Next is the "allocated" (duplicated) string and addressed in the
    // UpnpString.
    char mstring3[]{"set string"};

    StringMock mock_stringObj;
    mock::String string_injectObj(&mock_stringObj);
    EXPECT_CALL(mock_stringObj, strdup(mstring2)).WillOnce(Return(mstring3));

    mock::Stdlib stdlib_injectObj(&mock_stdlibObj);
    // The previous 'mstring1' should be freed before setting the new one.
    EXPECT_CALL(this->mock_stdlibObj, free(mstring1)).Times(1);

    // Test Unit
    EXPECT_PRED2(NS::UpnpString_set_String, p, mstring2);

    EXPECT_EQ(upnpstr.m_length, 10);
    EXPECT_EQ(upnpstr.m_string, mstring3);
    EXPECT_STREQ(upnpstr.m_string, "set string");
    EXPECT_STREQ(p->m_string, "set string");
}

TEST_F(UpnpStringMockTestSuite, set_upnp_string_n) {
    // provide an empty UpnpString
    char mstring_empty[]{""};
    UpnpString upnpstr{0, mstring_empty};
    UpnpString* p = &upnpstr;

    char mstring2[]{"hello world"}; // This string will set to the UpnpString.
    // Next is the "allocated" (duplicated) string and addressed in the
    // UpnpString.
    char mstring3[]{"hello world"};

    StringMock mock_stringObj;
    mock::String string_injectObj(&mock_stringObj);
    EXPECT_CALL(mock_stringObj, strndup(mstring2, 11))
        .WillOnce(Return(mstring3));

    mock::Stdlib stdlib_injectObj(&mock_stdlibObj);
    EXPECT_CALL(this->mock_stdlibObj, free(mstring_empty)).Times(1);

    // Test Unit
    EXPECT_PRED3(NS::UpnpString_set_StringN, p, mstring2, 11);

    EXPECT_EQ(upnpstr.m_length, 11);
    EXPECT_EQ(upnpstr.m_string, mstring3);
    EXPECT_STREQ(upnpstr.m_string, "hello world");
    EXPECT_STREQ(p->m_string, "hello world");

    // Empty string with lenth 0 should set an empty UpnpString.
    EXPECT_CALL(mock_stringObj, strndup(mstring_empty, 0))
        .WillOnce(Return(mstring_empty));
    EXPECT_CALL(this->mock_stdlibObj, free(mstring3)).Times(1);
    EXPECT_PRED3(NS::UpnpString_set_StringN, p, mstring_empty, 0);
    EXPECT_EQ(upnpstr.m_length, 0);
    EXPECT_STREQ(upnpstr.m_string, "");

    // longer string but length 0 should set an empty UpnpString.
    EXPECT_CALL(mock_stringObj, strndup(mstring2, 0))
        .WillOnce(Return(mstring_empty));
    EXPECT_CALL(this->mock_stdlibObj, free(mstring_empty)).Times(1);
    EXPECT_PRED3(NS::UpnpString_set_StringN, p, mstring2, 0);
    EXPECT_EQ(upnpstr.m_length, 0);
    EXPECT_STREQ(upnpstr.m_string, "");

    // longer string but length 1
    char mstring4[]{"h"};
    EXPECT_CALL(mock_stringObj, strndup(mstring2, 1))
        .WillOnce(Return(mstring4));
    EXPECT_CALL(this->mock_stdlibObj, free(mstring_empty)).Times(1);
    EXPECT_PRED3(NS::UpnpString_set_StringN, p, mstring2, 1);
    EXPECT_EQ(upnpstr.m_length, 1);
    EXPECT_EQ(upnpstr.m_string, mstring4);

    // Length = 10 is one character shorter than string length
    char mstring5[]{"hello worl"};
    EXPECT_CALL(mock_stringObj, strndup(mstring2, 10))
        .WillOnce(Return(mstring5));
    EXPECT_CALL(this->mock_stdlibObj, free(mstring4)).Times(1);
    EXPECT_PRED3(NS::UpnpString_set_StringN, p, mstring2, 10);
    EXPECT_EQ(upnpstr.m_length, 10);
    EXPECT_EQ(upnpstr.m_string, mstring5);

    // Length = 12 is one character longer than string length
    EXPECT_CALL(mock_stringObj, strndup(mstring2, 12))
        .WillOnce(Return(mstring3));
    EXPECT_CALL(this->mock_stdlibObj, free(mstring5)).Times(1);
    EXPECT_PRED3(NS::UpnpString_set_StringN, p, mstring2, 12);
    EXPECT_EQ(upnpstr.m_length, 11);
    EXPECT_EQ(upnpstr.m_string, mstring3);
}

// simple testsuite without fixtures
//----------------------------------
TEST(UpnpStringTestSuite, strnlen_called_with_various_parameter) {
    char str0[] = "";
    char str1[] = "123456789";

    EXPECT_EQ(strnlen(str0, 0), (size_t)0);
    EXPECT_EQ(strnlen(str0, 1), (size_t)0);
    EXPECT_EQ(str0[0], '\0'); // string terminator

    EXPECT_EQ(strnlen(str1, 0), (size_t)0);
    EXPECT_EQ(strnlen(str1, 1), (size_t)1);
    EXPECT_EQ(str1[1], '2');
    EXPECT_EQ(strnlen(str1, 10), (size_t)9);
    EXPECT_EQ(str1[9], '\0'); // string terminator
    EXPECT_EQ(strnlen(str1, 11), (size_t)9);
    str1[9] = 1; // destroy string terminator
    EXPECT_EQ(strnlen(str1, 10), (size_t)10);
}

TEST(UpnpStringTestSuite, strndup_called_with_various_parameter) {
    char str0[] = "";
    char str1[] = "123456789";
    std::string cpystr;

    cpystr = strndup(str0, 5);
    EXPECT_EQ(cpystr, "");
    EXPECT_EQ(cpystr[0], '\0'); // check string terminator

    cpystr = strndup(str1, 0);
    EXPECT_EQ(cpystr, "");
    EXPECT_EQ(cpystr[0], '\0'); // check string terminator

    cpystr = strndup(str1, 5);
    EXPECT_EQ(cpystr, "12345");

    cpystr = strndup(str1, 11);
    EXPECT_EQ(cpystr, "123456789");
    EXPECT_EQ(cpystr[9], '\0'); // check string terminator
}

TEST(UpnpStringTestSuite, clear_upnp_string) {
    // provide an UpnpString
    char mstring[]{"hello world"};
    UpnpString upnpstr{11, mstring};
    UpnpString* p = &upnpstr;

    // call the unit
    NS::UpnpString_clear(p);
    EXPECT_EQ(upnpstr.m_length, 0);
    EXPECT_STREQ(upnpstr.m_string, "");
}

// testsuite with death tests
//---------------------------
// Test suites with a name ending in “DeathTest” are run before all other tests.
// https://google.github.io/googletest/advanced.html#death-tests-and-threads
// This tests are general set in main() to be threadsafe (look at the end).

TEST(UpnpStringDeathTest, upnp_string_get_length) {
    // provide an UpnpString
    char mstring[] = "hello world";
    UpnpString upnpstr{11, mstring};
    UpnpString* p = &upnpstr;

    EXPECT_EQ(NS::UpnpString_get_Length(p), 11);

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " Function 'UpnpString_get_Length()' will segfault if "
                     "called with nullptr.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpString_get_Length(nullptr), ".*");

    } else {

        // No segfault but there should not be any changes on the UpnpString.
        EXPECT_EQ(NS::UpnpString_get_Length(nullptr), 0);

        EXPECT_EQ(upnpstr.m_length, 11);
        EXPECT_EQ(upnpstr.m_string, mstring);
    }
}

TEST(UpnpStringDeathTest, get_upnp_string) {
    // provide an UpnpString
    char mstring[] = "hello world";
    UpnpString upnpstr = {11, mstring};
    UpnpString* p = &upnpstr;

    EXPECT_STREQ(NS::UpnpString_get_String(p), "hello world");

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " Function 'UpnpString_get_String()' will segfault if "
                     "called with nullptr.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpString_get_String(nullptr), ".*");

    } else {

        // No segfault but there should not be any changes on the UpnpString.
        EXPECT_EQ(NS::UpnpString_get_String(nullptr), nullptr);
        EXPECT_EQ(upnpstr.m_length, 11);
        EXPECT_EQ(upnpstr.m_string, mstring);
    }
}

TEST(UpnpStringDeathTest, set_upnp_string_with_nullptr) {
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " Function 'UpnpString_set_String()' will segfault if "
                     "called with nullptr.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpString_set_String(nullptr, "hello world"), ".*");

    } else {

        // This expects NO segfault.
        EXPECT_EQ(NS::UpnpString_set_String(nullptr, "hello world"), 0);
    }
}

TEST(UpnpStringDeathTest, set_upnp_string_with_nullptr_to_string) {
    // provide an UpnpString
    char mstring[]{"hello world"};
    UpnpString upnpstr{11, mstring};
    UpnpString* p{&upnpstr};

    if (old_code) {
        // UpnpString_set_String() uses strdup() that segfaults on Unix
        // platforms but not on MS Windows if called with a nullptr.
#ifndef _WIN32
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " Function 'UpnpString_set_String()' will segfault if "
                     "called with nullptr to string.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpString_set_String(p, nullptr), ".*");
#else
        // This expects NO segfault.
        EXPECT_EXIT((NS::UpnpString_set_String(p, nullptr), exit(0)),
                    ExitedWithCode(0), ".*");
#endif
    } else {

        // No segfault but there should not be any changes on the UpnpString.
        EXPECT_EQ(NS::UpnpString_set_String(p, nullptr), 0);
        EXPECT_EQ(upnpstr.m_length, 11);
        EXPECT_EQ(upnpstr.m_string, mstring);
    }
}

TEST(UpnpStringDeathTest, set_upnp_string_n_with_nullptr) {
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " Function 'UpnpString_set_StringN()' will segfault "
                     "if called with nullptr.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpString_set_StringN(nullptr, "?", 1), ".*");

    } else {

        // This expects NO segfault.
        EXPECT_EQ(NS::UpnpString_set_StringN(nullptr, "?", 1), 0);
    }
}

TEST(UpnpStringDeathTest, set_upnp_string_n_with_nullptr_to_string) {
    // provide an UpnpString
    char mstring[]{"hello world"};
    UpnpString upnpstr{11, mstring};
    UpnpString* p{&upnpstr};

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " Function 'UpnpString_set_StringN()' will segfault "
                     "if called with nullptr to string.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpString_set_StringN(p, nullptr, 1), ".*");

    } else {

        // No segfault but there should not be any changes on the UpnpString.
        EXPECT_EQ(NS::UpnpString_set_StringN(p, nullptr, 1), 0);
        EXPECT_EQ(upnpstr.m_length, 11);
        EXPECT_EQ(upnpstr.m_string, mstring);
    }
}

TEST(UpnpStringDeathTest, clearUpnpString) {
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " Function 'UpnpString_clear()' will segfault if called "
                     "with nullptr.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpString_clear(nullptr), ".*");

    } else {

        // This expects NO segfault.
        EXPECT_EXIT((NS::UpnpString_clear(nullptr), exit(0)), ExitedWithCode(0),
                    ".*");
    }
}

} // namespace compa

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    // https://google.github.io/googletest/advanced.html#death-test-styles
#ifndef _WIN32
    // On MS Windows this flag isn't defined (error LNK2019: unresolved external
    // symbol). By default it behaves much like the "threadsafe" mode on POSIX
    // by default.
    // See https://google.github.io/googletest/reference/assertions.html#death
    // GTEST_FLAG_SET(death_test_style, "threadsafe");
#endif
#include "compa/gtest_main.inc"
}
