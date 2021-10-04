// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-10-02

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <filesystem>
#include <fstream>

#include "api/upnpdebug.cpp"

// clang-format off
#ifndef NDEBUG
  #error "NDEBUG is NOT defined"
#endif

#ifdef UPNP_DEBUG_C
  #ifdef _MSC_VER
    #pragma message("UPNP_DEBUG_C is defined")
  #else
    #warning "UPNP_DEBUG_C is defined"
  #endif
#else
  #ifdef _MSC_VER
    #pragma message("UPNP_DEBUG_C is NOT defined")
  #else
    #warning "UPNP_DEBUG_C is NOT defined"
  #endif
#endif
// clang-format on

using ::testing::MatchesRegex;

// Tests for the debugging and logging module
//-------------------------------------------
TEST(UpnpdebugTestSuite, display_file_and_line) {
    // generate random temporary filename
    std::srand(std::time(nullptr));
    std::string fname = std::filesystem::temp_directory_path().string() +
                        "/gtest" + std::to_string(std::rand());
    fp = fopen(fname.c_str(), "a");

    // process the unit that will write to the open fp
    UpnpDisplayFileAndLine(fp, "gtest_filename.dummy", 0, UPNP_ALL, API);
    fclose(fp);

    // look if the output is as expected
    std::ifstream file(fname);
    std::string str;
    std::getline(file, str);
    std::remove(fname.c_str());
    //"2021-04-21 10:05:38 UPNP-API_-3: Thread:0x7F998124D740
    //[gtest_filename.dummy:0]: "
    EXPECT_THAT(
        str, MatchesRegex(
                 "\\w\\w\\w\\w-\\w\\w-\\w\\w \\w\\w:\\w\\w:\\w\\w UPNP-API_-3: "
                 "Thread:0x\\w+ \\[gtest_filename.dummy:0\\]: "));
}

TEST(UpnpdebugTestSuite, setLogLevel) {
    UpnpSetLogLevel(UPNP_INFO);
    EXPECT_EQ(g_log_level, UPNP_INFO);
    EXPECT_EQ(setlogwascalled, 1);
}

TEST(UpnpdebugTestSuite, UpnpPrintf_valid_call) {
    initwascalled = 1;

    // generate random temporary filename
    std::srand(std::time(nullptr));
    std::string fname = std::filesystem::temp_directory_path().string() +
                        "/gtest" + std::to_string(std::rand());
    fp = fopen(fname.c_str(), "a");

    // process the unit that will write to the open fp
    std::cout << "I'm here 1\n";
    UpnpPrintf(UPNP_INFO, API, "gtest_filename.dummy", 0,
               "UpnpInit2 with IfName=%s, DestPort=%d.\n", "NULL", 51515);
    std::cout << "I'm here 2\n";
    fclose(fp);
    std::cout << "I'm here 3\n";

    // look if the output is as expected
    std::ifstream file(fname);
    std::string str;
    std::getline(file, str);
    std::remove(fname.c_str());
    //"2021-04-21 21:54:50 UPNP-API_-2: Thread:0x7FF8CF8C7740
    //[gtest_filename.dummy:0]: UpnpInit2 with IfName=NULL, DestPort=51515."
    EXPECT_THAT(
        str, MatchesRegex(
                 "\\w\\w\\w\\w-\\w\\w-\\w\\w \\w\\w:\\w\\w:\\w\\w UPNP-API_-3: "
                 "Thread:0x\\w+ \\[gtest_filename.dummy:0\\]: "));
}

TEST(UpnpdebugTestSuite, UpnpPrintf_not_initialized) {
    initwascalled = 0;

    // generate random temporary filename
    std::srand(std::time(nullptr));
    std::string fname = std::filesystem::temp_directory_path().string() +
                        "/gtest" + std::to_string(std::rand());
    fp = fopen(fname.c_str(), "a");

    // process the unit that will write to the open fp
    UpnpPrintf(UPNP_INFO, API, "gtest_filename.dummy", 0,
               "UpnpInit2 with IfName=%s, DestPort=%d.\n", "NULL", 51515);
    fclose(fp);

    // look if the output is as expected
    std::ifstream file(fname);
    std::string str;
    std::getline(file, str);
    std::remove(fname.c_str());

    // nothing happend
    EXPECT_EQ(str, "");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
