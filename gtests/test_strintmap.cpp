// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-12-31

#include "gtest/gtest.h"
//#include "gmock/gmock.h"

#include "genlib/util/strintmap.cpp"

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
TEST(StrintmapTestSuite, empty_test) {}

} // namespace upnp

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    // ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
