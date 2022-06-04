// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-06-05

// -----------------------------------------------------------------------------
// This testsuite starts the sample TV Device with general command line
// arguments and steps down the call stack with tests of the called functions so
// we can see what is needed to have the TV Device started. It is important to
// always stop the device after each test. Otherwise we get side effects on
// following tests.
// -----------------------------------------------------------------------------

#include "sample/common/tv_device.cpp"
#include "upnplib/upnptools.hpp"
#include "upnplib/cmake_vars.hpp"

#include "gtest/gtest.h"

namespace upnplib {

TEST(SampleTvDeviceTestSuite, invalid_commandline_argument) {
    constexpr int argc{5};
    char arg[argc][20]{"build/bin/tv_device", "-i", "ens2", "--webdir",
                       "./sample/web"};
    char* argv[argc]{arg[0], arg[1], arg[2], arg[3], arg[4]};

    // argc is 0 so the function should return immediately with error
    EXPECT_EQ(device_main(0, argv), UPNP_E_INVALID_ARGUMENT);

    // Stop device
    EXPECT_EQ(TvDeviceStop(), UPNP_E_SUCCESS);
}

TEST(SampleTvDeviceTestSuite, valid_arguments) {
    constexpr int argc{3};
    constexpr int argsize = sizeof(UPnPlib_BINARY_DIR "/bin/tv_device");
    char arg[argc][argsize]{UPnPlib_BINARY_DIR "/bin/tv_device", "-webdir",
                            UPNPLIB_SAMPLE_SOURCE_DIR "/web"};
    char* argv[argc]{arg[0], arg[1], arg[2]};

    // argc is valid with valid arguments
    EXPECT_EQ(device_main(argc, argv), UPNP_E_SUCCESS);

    // Stop device
    EXPECT_EQ(TvDeviceStop(), UPNP_E_SUCCESS);
}

TEST(SampleTvDeviceTestSuite, TvDeviceStart) {
    constexpr char* iface{};
    constexpr unsigned short port{};
    constexpr char* desc_doc_name{};
    constexpr char web_dir_path[]{UPNPLIB_SAMPLE_SOURCE_DIR "/web"};
    constexpr int ip_mode = IP_MODE_IPV4;

    // Start the TV Device with default settings as far as possible
    int rc = TvDeviceStart(iface, port, desc_doc_name, web_dir_path, ip_mode,
                           linux_print, 0);
    EXPECT_EQ(rc, UPNP_E_SUCCESS) << errStrEx(rc, UPNP_E_SUCCESS);

    // Stop device
    EXPECT_EQ(TvDeviceStop(), UPNP_E_SUCCESS);
}

TEST(SampleTvDeviceTestSuite, UpnpInit2) {
    // Initialize the library
    int rc = UpnpInit2(nullptr /*iface*/, 0 /*port*/);
    EXPECT_EQ(rc, UPNP_E_SUCCESS) << errStrEx(rc, UPNP_E_SUCCESS);

    // Finish library
    UpnpFinish();
}

} // namespace upnplib

//
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
