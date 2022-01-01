// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-01-01

#include "gtest/gtest.h"
//#include "gmock/gmock.h"

#include "genlib/util/strintmap.cpp"
#include "httpparser.hpp" // for HTTPMETHOD* constants

namespace upnp {

// Interface for the strintmap module
// ==================================
// clang-format off

class Istrintmap {
  public:
    virtual ~Istrintmap() {}

    virtual int map_str_to_int(
        const char* name, size_t name_len, str_int_entry* table, int num_entries, int case_sensitive) = 0;
    virtual int map_int_to_str(
        int id, str_int_entry* table, int num_entries) = 0;
};

class Cstrintmap : Istrintmap {
  public:
    virtual ~Cstrintmap() override {}

    int map_str_to_int(const char* name, size_t name_len, str_int_entry* table, int num_entries, int case_sensitive) override {
        return ::map_str_to_int(name, name_len, table, num_entries, case_sensitive); }
    int map_int_to_str(int id, str_int_entry* table, int num_entries) override {
        return ::map_int_to_str(id, table, num_entries); }
};
// clang-format on

//
// testsuite for strintmap
//========================

#define NUM_HTTP_METHODS 11
static str_int_entry Http_Method_Table[NUM_HTTP_METHODS] = {
    {"DELETE", HTTPMETHOD_DELETE},
    {"GET", HTTPMETHOD_GET},
    {"HEAD", HTTPMETHOD_HEAD},
    {"M-POST", HTTPMETHOD_MPOST},
    {"M-SEARCH", HTTPMETHOD_MSEARCH},
    {"NOTIFY", HTTPMETHOD_NOTIFY},
    {"POST", HTTPMETHOD_POST},
    {"SUBSCRIBE", HTTPMETHOD_SUBSCRIBE},
    {"UNSUBSCRIBE", HTTPMETHOD_UNSUBSCRIBE},
    {"POST", SOAPMETHOD_POST},
    {"PUT", HTTPMETHOD_PUT}};

//
class StrintmapTestSuite : public ::testing::Test {
  protected:
    Cstrintmap m_mapObj;
};

TEST_F(StrintmapTestSuite, map_str_to_int) {
    int idx = m_mapObj.map_str_to_int("HEAD", 4, Http_Method_Table,
                                      NUM_HTTP_METHODS, 1);
    EXPECT_EQ(idx, 2);
    // ::std::cout << "DEBUG: index = " << idx << ", HTTPMETHOD_HEAD = " <<
    // HTTPMETHOD_HEAD << ::std::endl;

    idx = m_mapObj.map_str_to_int("NOTIFY", 6, Http_Method_Table,
                                  NUM_HTTP_METHODS, 1);
    EXPECT_EQ(idx, 5);
    // ::std::cout << "DEBUG: index = " << idx << ", HTTPMETHOD_NOTIFY = " <<
    // HTTPMETHOD_NOTIFY << ::std::endl;
}

TEST_F(StrintmapTestSuite, map_str_to_int_with_nullptr_to_namestring) {
#ifdef OLD_TEST
    ::std::cout << "  BUG! A nullptr to the namestring must not segfault.\n";
#else
    int idx{0};
    ASSERT_EXIT((idx = m_mapObj.map_str_to_int(nullptr, 6, Http_Method_Table,
                                               NUM_HTTP_METHODS, 1),
                 exit(0)),
                ::testing::ExitedWithCode(0), ".*")
        << "  # A nullptr to the namestring must not segfault.";
    EXPECT_EQ(idx, -1);
#endif
}

TEST_F(StrintmapTestSuite, map_str_to_int_with_zero_namestring_length) {
    EXPECT_EQ(m_mapObj.map_str_to_int("NOTIFY", 0, Http_Method_Table,
                                      NUM_HTTP_METHODS, 1),
              -1);
}

TEST_F(StrintmapTestSuite, map_str_to_int_with_nullptr_to_table) {
#ifdef OLD_TEST
    ::std::cout << "  BUG! A nullptr to a table must not segfault.\n";
#else
    int idx{};
    ASSERT_EXIT((idx = m_mapObj.map_str_to_int("NOTIFY", 6, nullptr,
                                               NUM_HTTP_METHODS, 1),
                 exit(0)),
                ::testing::ExitedWithCode(0), ".*")
        << "  # A nullptr to a table must not segfault.";
    EXPECT_EQ(idx, -1);
#endif
}

TEST_F(StrintmapTestSuite, map_str_to_int_with_zero_table_entries) {
    EXPECT_EQ(m_mapObj.map_str_to_int("NOTIFY", 6, Http_Method_Table, 0, 1),
              -1);
}

TEST_F(StrintmapTestSuite, map_str_to_int_with_different_namestring_cases) {
    EXPECT_EQ(m_mapObj.map_str_to_int("Notify", 6, Http_Method_Table,
                                      NUM_HTTP_METHODS, -1),
              -1);
    EXPECT_EQ(m_mapObj.map_str_to_int("Notify", 6, Http_Method_Table,
                                      NUM_HTTP_METHODS, 0),
              5);
}

TEST_F(StrintmapTestSuite, map_int_to_str) {
    int idx = m_mapObj.map_int_to_str(::HTTPMETHOD_NOTIFY, Http_Method_Table,
                                      NUM_HTTP_METHODS);
    EXPECT_EQ(idx, 5);
    // ::std::cout << "DEBUG: name = " << Http_Method_Table[5].name << ", id = "
    // << Http_Method_Table[5].id << ::std::endl;
}

TEST_F(StrintmapTestSuite, map_int_to_str_with_invalid_id) {
    int idx{};
    idx = m_mapObj.map_int_to_str(65444, Http_Method_Table, NUM_HTTP_METHODS);
    EXPECT_EQ(idx, -1);
}

TEST_F(StrintmapTestSuite, map_int_to_str_with_nullptr_to_table) {
#ifdef OLD_TEST
    ::std::cout << "  BUG! A nullptr to a table must not segfault.\n";
#else
    int idx{};
    ASSERT_EXIT((idx = m_mapObj.map_int_to_str(::HTTPMETHOD_NOTIFY, nullptr,
                                               NUM_HTTP_METHODS),
                 exit(0)),
                ::testing::ExitedWithCode(0), ".*")
        << "  # A nullptr to a table must not segfault.";
    EXPECT_EQ(idx, -1);
#endif
}

TEST_F(StrintmapTestSuite, map_int_to_str_with_zero_table_entiries) {
    int idx{};
    idx = m_mapObj.map_int_to_str(::HTTPMETHOD_NOTIFY, Http_Method_Table, 0);
    EXPECT_EQ(idx, -1);
}

TEST_F(StrintmapTestSuite, map_int_to_str_with_oversized_table_entiries) {
    int idx{};
    idx =
        m_mapObj.map_int_to_str(65444, Http_Method_Table, NUM_HTTP_METHODS + 1);
    EXPECT_EQ(idx, -1);
}

} // namespace upnp

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    // ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
