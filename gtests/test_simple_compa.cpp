// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-05-23
//

#ifdef UPNPLIB_WITH_NATIVE_PUPNP
#include "UpnpGlobal.hpp"
#else
#include "compa/global.hpp"
#endif
#include "upnplib/upnptools.hpp"

#include "gtest/gtest.h"

using upnplib::errStr;

//
// simple testsuite
//-----------------
#ifdef UPNPLIB_WITH_NATIVE_PUPNP
TEST(simpleTestSuite, version_of_pupnp_native_library) {
    EXPECT_STREQ(library_version, "pupnp_native 1.14.12");
}
#else

TEST(simpleTestSuite, version_of_upnplib_compa_library) {
    EXPECT_STREQ(library_version, "upnplib_compa 1.14.12");
}
#endif

TEST(simpleTestSuite, simple_upnplib_native_test) {
    EXPECT_EQ(errStr(0), "UPNP_E_SUCCESS(0)");
}

//
// main entry
// ----------
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
