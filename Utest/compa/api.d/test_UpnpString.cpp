// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-01-14

#include <UpnpString.hpp>
#if defined UPNPLIB_WITH_NATIVE_PUPNP && !defined PUPNP_UPNPSTRING_HPP
#error "Wrong UpnpString.hpp header file included for PUPNP"
#endif
#if !defined UPNPLIB_WITH_NATIVE_PUPNP && !defined COMPA_UPNPSTRING_HPP
#error "Wrong UpnpString.hpp header file included for COMPA"
#endif

#ifdef UPNPLIB_WITH_NATIVE_PUPNP
#define NS
#else
#define NS
#endif

#include <utest/utest.hpp>
#include <umock/stdlib_mock.hpp>
#include <umock/stringh_mock.hpp>

using ::testing::_;
using ::testing::Eq;
using ::testing::ExitedWithCode;
using ::testing::Return;
using ::testing::StrictMock;


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

namespace utest {

class UpnpStringFTestSuite : public ::testing::Test {
  protected:
    // Instantiate mocking objects.
    StrictMock<umock::StringhMock> m_stringhObj;
    // Inject the mocking objects into the tested code.
    umock::Stringh stringh_injectObj = umock::Stringh(&m_stringhObj);
};

// Testsuite
// =========
TEST(UpnpStringMockTestSuite, create_new_upnp_string) {
    // provide a structure of a UpnpString
    char mstring[]{""};
    UpnpString upnpstr{};
    UpnpString* p = &upnpstr;
    UpnpString* str{};

    umock::StdlibMock stdlibObj;
    umock::Stdlib stdlib_injectObj(&stdlibObj);
    EXPECT_CALL(stdlibObj, calloc(_, _))
        .WillOnce(Return(p))
        .WillOnce(Return(mstring));

    // Test Unit
    str = UpnpString_new();

    // str should point to the upnpstr structure
    EXPECT_EQ(str, &upnpstr);
    EXPECT_EQ(str->m_length, (size_t)0);
    // and its member 'm_string' should point to an empty string (mstring) that
    // is also allocated on the heap (if not mocked like here).
    EXPECT_EQ(str->m_string, mstring);
    EXPECT_EQ(upnpstr.m_string, mstring);
    EXPECT_STREQ(str->m_string, "");

    // test edge conditions
    EXPECT_CALL(stdlibObj, calloc(_, _)).WillOnce(Return(nullptr));

    // Test Unit
    str = UpnpString_new();
    EXPECT_EQ(str, nullptr);

    EXPECT_CALL(stdlibObj, calloc(_, _))
        .WillOnce(Return(p))
        .WillOnce(Return(nullptr));
    EXPECT_CALL(stdlibObj, free(_)).Times(1);

    // Test Unit
    str = UpnpString_new();
    EXPECT_EQ(str, nullptr);
}

TEST(UpnpStringMockTestSuite, delete_upnp_string) {
    // provide an UpnpString
    char mstring[] = "hello world";
    UpnpString upnpstr = {11, mstring};
    UpnpString* p = &upnpstr;

    // Test Unit, check edge condition
    UpnpString_delete(nullptr);

    EXPECT_EQ(upnpstr.m_length, (size_t)11);
    EXPECT_STREQ(upnpstr.m_string, "hello world");

    umock::StdlibMock stdlibObj;
    umock::Stdlib stdlib_injectObj(&stdlibObj);
    EXPECT_CALL(stdlibObj, free(_)).Times(2);

    // Test Unit
    UpnpString_delete(p);

    EXPECT_EQ(upnpstr.m_length, (size_t)0);
    EXPECT_EQ(upnpstr.m_string, nullptr);
}

TEST_F(UpnpStringFTestSuite, set_upnp_string) {
    // provide an empty UpnpString
    char mstring1[]{""};
    UpnpString upnpstr{0, mstring1};
    UpnpString* p = &upnpstr;

    char mstring2[]{"set string"}; // This string will be set to the UpnpString.
    // Next is the "allocated" (duplicated) string and addressed in the
    // UpnpString.
    char mstring3[]{"set string"};

    EXPECT_CALL(m_stringhObj, strdup(mstring2)).WillOnce(Return(mstring3));

    umock::StdlibMock stdlibObj;
    umock::Stdlib stdlib_injectObj(&stdlibObj);
    // The previous 'mstring1' should be freed before setting the new one.
    EXPECT_CALL(stdlibObj, free(mstring1)).Times(1);

    // Test Unit
    EXPECT_PRED2(NS::UpnpString_set_String, p, mstring2);

    EXPECT_EQ(upnpstr.m_length, (size_t)10);
    EXPECT_EQ(upnpstr.m_string, mstring3);
    EXPECT_STREQ(upnpstr.m_string, "set string");
    EXPECT_STREQ(p->m_string, "set string");
}

TEST_F(UpnpStringFTestSuite, set_upnp_string_n) {
    // provide an empty UpnpString
    char mstring_empty[]{""};
    UpnpString upnpstr{0, mstring_empty};
    UpnpString* p = &upnpstr;

    char mstring2[]{"hello world"}; // This string will set to the UpnpString.
    // Next is the "allocated" (duplicated) string and addressed in the
    // UpnpString.
    char mstring3[]{"hello world"};

    EXPECT_CALL(m_stringhObj, strndup(mstring2, 11)).WillOnce(Return(mstring3));

    umock::StdlibMock stdlibObj;
    umock::Stdlib stdlib_injectObj(&stdlibObj);
    EXPECT_CALL(stdlibObj, free(mstring_empty)).Times(1);

    // Test Unit
    EXPECT_PRED3(NS::UpnpString_set_StringN, p, mstring2, 11);

    EXPECT_EQ(upnpstr.m_length, (size_t)11);
    EXPECT_EQ(upnpstr.m_string, mstring3);
    EXPECT_STREQ(upnpstr.m_string, "hello world");
    EXPECT_STREQ(p->m_string, "hello world");

    // Empty string with lenth 0 should set an empty UpnpString.
    EXPECT_CALL(m_stringhObj, strndup(mstring_empty, 0))
        .WillOnce(Return(mstring_empty));
    EXPECT_CALL(stdlibObj, free(mstring3)).Times(1);
    EXPECT_PRED3(NS::UpnpString_set_StringN, p, mstring_empty, 0);
    EXPECT_EQ(upnpstr.m_length, (size_t)0);
    EXPECT_STREQ(upnpstr.m_string, "");

    // longer string but length 0 should set an empty UpnpString.
    EXPECT_CALL(m_stringhObj, strndup(mstring2, 0))
        .WillOnce(Return(mstring_empty));
    EXPECT_CALL(stdlibObj, free(mstring_empty)).Times(1);
    EXPECT_PRED3(NS::UpnpString_set_StringN, p, mstring2, 0);
    EXPECT_EQ(upnpstr.m_length, (size_t)0);
    EXPECT_STREQ(upnpstr.m_string, "");

    // longer string but length 1
    char mstring4[]{"h"};
    EXPECT_CALL(m_stringhObj, strndup(mstring2, 1)).WillOnce(Return(mstring4));
    EXPECT_CALL(stdlibObj, free(mstring_empty)).Times(1);
    EXPECT_PRED3(NS::UpnpString_set_StringN, p, mstring2, 1);
    EXPECT_EQ(upnpstr.m_length, (size_t)1);
    EXPECT_EQ(upnpstr.m_string, mstring4);

    // Length = 10 is one character shorter than string length
    char mstring5[]{"hello worl"};
    EXPECT_CALL(m_stringhObj, strndup(mstring2, 10)).WillOnce(Return(mstring5));
    EXPECT_CALL(stdlibObj, free(mstring4)).Times(1);
    EXPECT_PRED3(NS::UpnpString_set_StringN, p, mstring2, 10);
    EXPECT_EQ(upnpstr.m_length, (size_t)10);
    EXPECT_EQ(upnpstr.m_string, mstring5);

    // Length = 12 is one character longer than string length
    EXPECT_CALL(m_stringhObj, strndup(mstring2, 12)).WillOnce(Return(mstring3));
    EXPECT_CALL(stdlibObj, free(mstring5)).Times(1);
    EXPECT_PRED3(NS::UpnpString_set_StringN, p, mstring2, 12);
    EXPECT_EQ(upnpstr.m_length, (size_t)11);
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
    // Not possible to test at runtime here due to compiler error:
    // ‘size_t strnlen(const char*, size_t)’ specified bound 11 exceeds source
    // size 10 [-Werror=stringop-overread]
    // EXPECT_EQ(strnlen(str1, 11), (size_t)9);
    str1[9] = 1; // destroy string terminator
    EXPECT_EQ(strnlen(str1, 10), (size_t)10);
}

TEST(UpnpStringTestSuite, strndup_called_with_various_parameter) {
    [[maybe_unused]] char str0[] = "";
    char str1[] = "123456789";
    std::string cpystr;

    // Not possible to test at runtime here due to compiler error.
    // cpystr = strndup(str0, 5);
    // EXPECT_EQ(cpystr, "");
    // EXPECT_EQ(cpystr[0], '\0'); // check string terminator

    cpystr = strndup(str1, 0);
    EXPECT_EQ(cpystr, "");
    EXPECT_EQ(cpystr[0], '\0'); // check string terminator

    cpystr = strndup(str1, 5);
    EXPECT_EQ(cpystr, "12345");

    // Not possible to test at runtime here due to compiler error.
    // cpystr = strndup(str1, 11);
    // EXPECT_EQ(cpystr, "123456789");
    // EXPECT_EQ(cpystr[9], '\0'); // check string terminator
}

TEST(UpnpStringTestSuite, clear_upnp_string) {
    // provide an UpnpString
    char mstring[]{"hello world"};
    UpnpString upnpstr{11, mstring};
    UpnpString* p = &upnpstr;

    // call the unit
    NS::UpnpString_clear(p);
    EXPECT_EQ(upnpstr.m_length, (size_t)0);
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

    EXPECT_EQ(NS::UpnpString_get_Length(p), (size_t)11);

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " Function 'UpnpString_get_Length()' will segfault if "
                     "called with nullptr.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpString_get_Length(nullptr), ".*");

    } else {

        // No segfault but there should not be any changes on the UpnpString.
        EXPECT_EQ(NS::UpnpString_get_Length(nullptr), (size_t)0);

        EXPECT_EQ(upnpstr.m_length, (size_t)11);
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
        EXPECT_EQ(upnpstr.m_length, (size_t)11);
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
        EXPECT_EQ(upnpstr.m_length, (size_t)11);
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
        EXPECT_EQ(upnpstr.m_length, (size_t)11);
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

} // namespace utest

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
#include <utest/utest_main.inc>
    return gtest_return_code; // managed in gtest_main.inc
}
