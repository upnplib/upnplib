// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-05-23

#include "upnplib/upnptools.hpp"
#include "upnp.hpp"

#include "gtest/gtest.h"

//
namespace upnplib {

TEST(UpnptoolsTestSuite, get_error_string) {

    EXPECT_STREQ(err_c_str(UPNP_E_INVALID_PARAM), "UPNP_E_INVALID_PARAM");
    EXPECT_STREQ(err_c_str(481895), "Unknown error code");

    EXPECT_EQ(errStr(0), "UPNP_E_SUCCESS(0)");
    EXPECT_EQ(errStr(UPNP_E_INTERNAL_ERROR), "UPNP_E_INTERNAL_ERROR(-911)");
    EXPECT_EQ(errStr(480895), "UPNPLIB_E_UNKNOWN(480895)");
}

TEST(UpnptoolsTestSuite, get_extended_error_string) {

    EXPECT_EQ(errStrEx(UPNP_E_INVALID_PARAM, UPNP_E_SUCCESS),
              "  # Should be UPNP_E_SUCCESS(0), but not "
              "UPNP_E_INVALID_PARAM(-101).");

    EXPECT_EQ(errStrEx(489895, UPNP_E_INTERNAL_ERROR),
              "  # Should be UPNP_E_INTERNAL_ERROR(-911), but not "
              "UPNPLIB_E_UNKNOWN(489895).");

    EXPECT_EQ(errStrEx(UPNP_E_INTERNAL_ERROR, -168494),
              "  # Should be UPNPLIB_E_UNKNOWN(-168494), but not "
              "UPNP_E_INTERNAL_ERROR(-911).");

    EXPECT_EQ(errStrEx(99328, -168494),
              "  # Should be UPNPLIB_E_UNKNOWN(-168494), but not "
              "UPNPLIB_E_UNKNOWN(99328).");
}

} // namespace upnplib

// main entry
// ----------
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
