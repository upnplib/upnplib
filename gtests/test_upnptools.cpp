// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-03-07

#include "gtest/gtest.h"
#include "core/src/api/upnptools.cpp"

//
namespace upnplib {
bool old_code{false};

TEST(UpnptoolsTestSuite, get_error_string) {
    if (old_code) {

        EXPECT_STREQ(UpnpGetErrorMessage(UPNP_E_INVALID_PARAM),
                     "UPNP_E_INVALID_PARAM");
        EXPECT_STREQ(UpnpGetErrorMessage(480895), "Unknown error code");
    } else {

        EXPECT_STREQ(errCstr(UPNP_E_INVALID_PARAM), "UPNP_E_INVALID_PARAM");
        EXPECT_STREQ(errCstr(481895), "Unknown error code");
        EXPECT_EQ(errStrEx(UPNP_E_INVALID_PARAM), "UPNP_E_INVALID_PARAM(-101)");
        EXPECT_EQ(errStrEx(489895), "Unknown error code(489895)");
    }
}

} // namespace upnplib

//
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
#include "upnplib/gtest_main.inc"
}
