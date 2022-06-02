// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-06-02

#include "gmock/gmock.h"
#include "gtest/gtest.h"

//#undef HAVE_STRNLEN
//#undef HAVE_STRNDUP

using ::testing::_;
using ::testing::Eq;
using ::testing::Return;

// --- mock calloc ---------------------------------------
class CMock_calloc {
  public:
    MOCK_METHOD(void*, calloc, (size_t nmemb, size_t size));
};
CMock_calloc* ptrMock_calloc = nullptr;

// --- mock free -----------------------------------------
class CMock_free {
  public:
    MOCK_METHOD(void, free, (void* ptr));
};
CMock_free* ptrMock_free = nullptr;

namespace { // No name for file scope. This is the C++ way for decorator STATIC.

// The following functions overwrite the functions from the system library.
// This is only possible with a local (file) scope of these functions. Also they
// must be defined before the included program that is tested.

void* calloc(size_t nmemb, size_t size) {
    return ptrMock_calloc->calloc(nmemb, size);
}
void free(void* ptr) { return ptrMock_free->free(ptr); }

#include "pupnp/upnp/src/api/UpnpString.cpp"

} // namespace

// testsuite with fixtures
//------------------------
class UpnpStringMockTestSuite : public ::testing::Test {
  protected:
    // Instantiate the mock objects.
    // The global pointer to them are set in the constructor below.
    CMock_calloc mock_calloc;
    CMock_free mock_free;

    UpnpStringMockTestSuite() {
        // set the global pointer to the mock objects
        ptrMock_calloc = &mock_calloc;
        ptrMock_free = &mock_free;
    }
};

TEST_F(UpnpStringMockTestSuite, createNewUpnpString) {
    // provide a structure of a UpnpString
    char mstring[] = {0};
    SUpnpString upnpstr = {};
    UpnpString* p = (UpnpString*)&upnpstr;
    UpnpString* str;

    EXPECT_CALL(mock_calloc, calloc(_, _))
        .WillOnce(Return(p))
        .WillOnce(Return(*&mstring));
    // call the unit
    str = UpnpString_new();
    EXPECT_THAT(str, Eq(p));

    // test edge conditions
    EXPECT_CALL(mock_calloc, calloc(_, _)).WillOnce(Return((UpnpString*)NULL));
    // call the unit
    str = UpnpString_new();
    EXPECT_EQ(str, (UpnpString*)NULL);

    EXPECT_CALL(mock_calloc, calloc(_, _))
        .WillOnce(Return(p))
        .WillOnce(Return((char*)NULL));
    EXPECT_CALL(mock_free, free(_)).Times(1);
    // call the unit
    str = UpnpString_new();
    EXPECT_EQ(str, (UpnpString*)NULL);
}

TEST_F(UpnpStringMockTestSuite, deleteUpnpString) {
    // provide a UpnpString
    char mstring[] = "hello world";
    SUpnpString upnpstr = {11, *&mstring};
    UpnpString* p = (UpnpString*)&upnpstr;

    // Check edge condition
    UpnpString_delete((UpnpString*)nullptr);
    EXPECT_EQ(upnpstr.m_length, (size_t)11);
    EXPECT_STREQ(upnpstr.m_string, "hello world");

    // call the unit
    EXPECT_CALL(mock_free, free(_)).Times(2);
    UpnpString_delete(p);
    EXPECT_EQ(upnpstr.m_length, (size_t)0);
    EXPECT_EQ(upnpstr.m_string, (char*)NULL);
}

TEST_F(UpnpStringMockTestSuite, setUpnpString) {
    // provide an empty UpnpString
    char mstring[] = {0};
    SUpnpString upnpstr = {0, *&mstring};
    UpnpString* p = (UpnpString*)&upnpstr;

    // call the unit
    EXPECT_CALL(mock_free, free(_)).Times(1);
    EXPECT_PRED2(UpnpString_set_String, p, (const char*)"set string");
    EXPECT_EQ(upnpstr.m_length, (size_t)10);
    EXPECT_STREQ(upnpstr.m_string, "set string");
}

TEST_F(UpnpStringMockTestSuite, setUpnpStringN) {
    // provide an empty UpnpString
    char mstring[] = {0};
    SUpnpString upnpstr = {0, *&mstring};
    UpnpString* p = (UpnpString*)&upnpstr;

    // call the unit
    EXPECT_CALL(mock_free, free(_)).Times(1);
    EXPECT_PRED3(UpnpString_set_StringN, p, (const char*)"hello world", 0);
    EXPECT_EQ(upnpstr.m_length, (size_t)0);
    EXPECT_STREQ(upnpstr.m_string, "");

    EXPECT_CALL(mock_free, free(_)).Times(1);
    EXPECT_PRED3(UpnpString_set_StringN, p, (const char*)"hello world", 1);
    EXPECT_EQ(upnpstr.m_length, (size_t)1);
    EXPECT_STREQ(upnpstr.m_string, "h");

    EXPECT_CALL(mock_free, free(_)).Times(1);
    EXPECT_PRED3(UpnpString_set_StringN, p, (const char*)"hello world", 10);
    EXPECT_EQ(upnpstr.m_length, (size_t)10);
    EXPECT_STREQ(upnpstr.m_string, "hello worl");

    EXPECT_CALL(mock_free, free(_)).Times(1);
    EXPECT_PRED3(UpnpString_set_StringN, p, (const char*)"hello world", 11);
    EXPECT_EQ(upnpstr.m_length, (size_t)11);
    EXPECT_STREQ(upnpstr.m_string, "hello world");

    EXPECT_CALL(mock_free, free(_)).Times(1);
    EXPECT_PRED3(UpnpString_set_StringN, p, (const char*)"hello world", 12);
    EXPECT_EQ(upnpstr.m_length, (size_t)11);
    EXPECT_STREQ(upnpstr.m_string, "hello world");
}

// simple testsuite without fixtures
//----------------------------------
TEST(UpnpStringTestSuite, strnlenCalledWithVariousParameter) {
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
#ifndef UPNPLIB_WITH_NATIVE_PUPNP
    ADD_FAILURE() << "# strnlen isn't used in the UPnPlib core.";
#endif
}

TEST(UpnpStringTestSuite, strndupCalledWithVariousParameter) {
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
#ifndef UPNPLIB_WITH_NATIVE_PUPNP
    ADD_FAILURE() << "# strndup isn't used in the UPnPlib core.";
#endif
}

TEST(UpnpStringTestSuite, clearUpnpString) {
    // provide a UpnpString
    char mstring[] = "hello world";
    SUpnpString upnpstr = {11, *&mstring};
    UpnpString* p = (UpnpString*)&upnpstr;

    // call the unit
    UpnpString_clear(p);
    EXPECT_EQ(upnpstr.m_length, (size_t)0);
    EXPECT_STREQ(upnpstr.m_string, "");
}

// testsuite with death tests
//---------------------------
// Test suites with a name ending in “DeathTest” are run before all other tests.
// https://google.github.io/googletest/advanced.html#death-tests-and-threads
// This tests are general set in main() to be threadsafe (look at the end).

TEST(UpnpStringDeathTest, UpnpStringGetLength) {
    // provide a UpnpString
    char mstring[] = "hello world";
    SUpnpString upnpstr = {11, *&mstring};
    UpnpString* p = (UpnpString*)&upnpstr;

    EXPECT_EQ(UpnpString_get_Length(p), (size_t)11);

#ifdef UPNPLIB_WITH_NATIVE_PUPNP
    std::cout << "  BUG! Function 'UpnpString_get_Length()' will segfault if "
                 "called with nullptr.\n";
#else
    // call the unit with NULL pointer
    EXPECT_DEATH(UpnpString_get_Length(nullptr), "");
#endif
}

TEST(UpnpStringDeathTest, getUpnpString) {
    // provide a UpnpString
    char mstring[] = "hello world";
    SUpnpString upnpstr = {11, *&mstring};
    UpnpString* p = (UpnpString*)&upnpstr;

    EXPECT_STREQ(UpnpString_get_String(p), "hello world");

#ifdef UPNPLIB_WITH_NATIVE_PUPNP
    std::cout << "  BUG! Function 'UpnpString_get_String()' will segfault if "
                 "called with nullptr.\n";
#else
    // call the unit with NULL pointer
    EXPECT_DEATH(UpnpString_get_String((UpnpString*)nullptr), "");
#endif
}

TEST(UpnpStringDeathTest, setUpnpString) {
#ifdef UPNPLIB_WITH_NATIVE_PUPNP
    std::cout << "  BUG! Function 'UpnpString_set_String()' will segfault if "
                 "called with nullptr.\n";
#else
    // provide a UpnpString
    char mstring[] = "hello world";
    SUpnpString upnpstr = {11, *&mstring};
    UpnpString* p = (UpnpString*)&upnpstr;

    // call the unit with NULL pointer
    EXPECT_DEATH(UpnpString_set_String(nullptr, nullptr), "Segmentation fault");
    EXPECT_DEATH(UpnpString_set_String(nullptr, *&"another string"),
                 "Segmentation fault");
    EXPECT_DEATH(UpnpString_set_String(p, nullptr), "Segmentation fault");
#endif
}

TEST(UpnpStringDeathTest, setUpnpStringN) {
#ifdef UPNPLIB_WITH_NATIVE_PUPNP
    std::cout << "  BUG! Function 'UpnpString_set_StringN()' will segfault if "
                 "called with nullptr.\n";
#else
    // provide a UpnpString
    char mstring[] = "hello world";
    SUpnpString upnpstr = {11, *&mstring};
    UpnpString* p = (UpnpString*)&upnpstr;

    // call the unit with NULL pointer
    EXPECT_DEATH(UpnpString_set_StringN(nullptr, nullptr, 0),
                 "Segmentation fault");
    EXPECT_DEATH(UpnpString_set_StringN(nullptr, *&"another string", 0),
                 "Segmentation fault");
    EXPECT_DEATH(UpnpString_set_StringN(p, nullptr, 0), "Segmentation fault");
#endif
}

TEST(UpnpStringDeathTest, clearUpnpString) {
#ifdef UPNPLIB_WITH_NATIVE_PUPNP
    std::cout << "  BUG! Function 'UpnpString_clear()' will segfault if called "
                 "with nullptr.\n";
#else
    // call the unit with NULL pointer
    EXPECT_DEATH(UpnpString_clear(nullptr), "Segmentation fault");
#endif
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    // https://google.github.io/googletest/advanced.html#death-test-styles
#ifndef _WIN32
    // On MS Windows this flag isn't defined (error LNK2019: unresolved external
    // symbol). By default it behaves much like the "threadsafe" mode on POSIX
    // by default.
    // See https://google.github.io/googletest/reference/assertions.html#death
    // GTEST_FLAG_SET(death_test_style, "threadsafe");
#endif
    return RUN_ALL_TESTS();
}
