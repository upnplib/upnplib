// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-09-04

#include "api/UpnpString.cpp"
#include "gtest/gtest.h"

#undef HAVE_STRNLEN
#undef HAVE_STRNDUP

void free(void* ptr) {}
// This function overwrites the 'free' function from the system library that
// frees allocated memory from the heap. The test settings does not use memory
// on the heap so this dummy function just do nothing.

// simple testsuite without fixtures
//----------------------------------
TEST(UpnpStringTestSuite, strnlenCalledWithVariousParameter) {
    char str0[] = "";
    char str1[] = "123456789";

    EXPECT_EQ(strnlen(str0, 0), 0);
    EXPECT_EQ(strnlen(str0, 1), 0);
    EXPECT_EQ(str0[0], '\0'); // string terminator

    EXPECT_EQ(strnlen(str1, 0), 0);
    EXPECT_EQ(strnlen(str1, 1), 1);
    EXPECT_EQ(str1[1], '2');
    EXPECT_EQ(strnlen(str1, 10), 9);
    EXPECT_EQ(str1[9], '\0'); // string terminator
    EXPECT_EQ(strnlen(str1, 11), 9);
    str1[9] = 1; // destroy string terminator
    EXPECT_EQ(strnlen(str1, 10), 10);
#ifndef OLD_TEST
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
#ifndef OLD_TEST
    ADD_FAILURE() << "# strndup isn't used in the UPnPlib core.";
#endif
}

TEST(UpnpStringTestSuite, deleteUpnpString) {
    // provide a UpnpString
    char mstring[] = "hello world";
    SUpnpString upnpstr = {11, *&mstring};
    UpnpString* p = (UpnpString*)&upnpstr;

    // Check edge condition
    UpnpString_delete((UpnpString*)nullptr);
    EXPECT_EQ(upnpstr.m_length, 11);
    EXPECT_STREQ(upnpstr.m_string, "hello world");

    // call the unit
    UpnpString_delete(p);
    EXPECT_EQ(upnpstr.m_length, 0);
    EXPECT_EQ(upnpstr.m_string, (char*)NULL);
}

TEST(UpnpStringTestSuite, setUpnpString) {
    // provide an empty UpnpString
    char mstring[] = {};
    SUpnpString upnpstr = {0, *&mstring};
    UpnpString* p = (UpnpString*)&upnpstr;

    // call the unit
    EXPECT_PRED2(UpnpString_set_String, p, *&"set string");
    EXPECT_EQ(upnpstr.m_length, 10);
    EXPECT_STREQ(upnpstr.m_string, "set string");
}

TEST(UpnpStringTestSuite, setUpnpStringN) {
    // provide an empty UpnpString
    char mstring[] = {};
    SUpnpString upnpstr = {0, *&mstring};
    UpnpString* p = (UpnpString*)&upnpstr;

    // call the unit
    EXPECT_PRED3(UpnpString_set_StringN, p, *&"hello world", 0);
    EXPECT_EQ(upnpstr.m_length, 0);
    EXPECT_STREQ(upnpstr.m_string, "");

    EXPECT_PRED3(UpnpString_set_StringN, p, *&"hello world", 1);
    EXPECT_EQ(upnpstr.m_length, 1);
    EXPECT_STREQ(upnpstr.m_string, "h");

    EXPECT_PRED3(UpnpString_set_StringN, p, *&"hello world", 10);
    EXPECT_EQ(upnpstr.m_length, 10);
    EXPECT_STREQ(upnpstr.m_string, "hello worl");

    EXPECT_PRED3(UpnpString_set_StringN, p, *&"hello world", 11);
    EXPECT_EQ(upnpstr.m_length, 11);
    EXPECT_STREQ(upnpstr.m_string, "hello world");

    EXPECT_PRED3(UpnpString_set_StringN, p, *&"hello world", 12);
    EXPECT_EQ(upnpstr.m_length, 11);
    EXPECT_STREQ(upnpstr.m_string, "hello world");
}

TEST(UpnpStringTestSuite, clearUpnpString) {
    // provide a UpnpString
    char mstring[] = "hello world";
    SUpnpString upnpstr = {11, *&mstring};
    UpnpString* p = (UpnpString*)&upnpstr;

    // call the unit
    UpnpString_clear(p);
    EXPECT_EQ(upnpstr.m_length, 0);
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

    EXPECT_EQ(UpnpString_get_Length(p), 11);

#ifdef OLD_TEST
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

#ifdef OLD_TEST
    std::cout << "  BUG! Function 'UpnpString_get_String()' will segfault if "
                 "called with nullptr.\n";
#else
    // call the unit with NULL pointer
    EXPECT_DEATH(UpnpString_get_String((UpnpString*)nullptr), "");
#endif
}

TEST(UpnpStringDeathTest, setUpnpString) {
    // provide a UpnpString
    char mstring[] = "hello world";
    SUpnpString upnpstr = {11, *&mstring};
    UpnpString* p = (UpnpString*)&upnpstr;

#ifdef OLD_TEST
    std::cout << "  BUG! Function 'UpnpString_set_String()' will segfault if "
                 "called with nullptr.\n";
#else
    // call the unit with NULL pointer
    EXPECT_DEATH(UpnpString_set_String(nullptr, nullptr), "Segmentation fault");
    EXPECT_DEATH(UpnpString_set_String(nullptr, *&"another string"),
                 "Segmentation fault");
    EXPECT_DEATH(UpnpString_set_String(p, nullptr), "Segmentation fault");
#endif
}

TEST(UpnpStringDeathTest, setUpnpStringN) {
    // provide a UpnpString
    char mstring[] = "hello world";
    SUpnpString upnpstr = {11, *&mstring};
    UpnpString* p = (UpnpString*)&upnpstr;

#ifdef OLD_TEST
    std::cout << "  BUG! Function 'UpnpString_set_StringN()' will segfault if "
                 "called with nullptr.\n";
#else
    // call the unit with NULL pointer
    EXPECT_DEATH(UpnpString_set_StringN(nullptr, nullptr, 0),
                 "Segmentation fault");
    EXPECT_DEATH(UpnpString_set_StringN(nullptr, *&"another string", 0),
                 "Segmentation fault");
    EXPECT_DEATH(UpnpString_set_StringN(p, nullptr, 0), "Segmentation fault");
#endif
}

TEST(UpnpStringDeathTest, clearUpnpString) {
    // provide a UpnpString
    char mstring[] = "hello world";
    SUpnpString upnpstr = {11, *&mstring};
    UpnpString* p = (UpnpString*)&upnpstr;

#ifdef OLD_TEST
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
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    return RUN_ALL_TESTS();
}
