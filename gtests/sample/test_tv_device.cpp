// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-06-29

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

// The tv_device_main call stack to use the pupnp library
//=======================================================
/*
clang-format off

     main() // Starting in tv_device_main.cpp
     |__ device_main()
     |   |__ SampleUtil_Initialize()
     |   |__ 'Parse command line options'
     |   |__ TvDeviceStart()
     |       |__ pthread_mutex_init()
     |       |__ UpnpSetLogFileNames()
     |       |__ UpnpSetLogLevel()
     |       |__ UpnpInitLog()
     |       |
     |       |__ UpnpInit2()
     |       |__ if error
     |              UpnpFinish()
     |       |
     |       |__ switch ip_mode
     |       |     UpnpGetServerIpAddress()
     |       |     UpnpGetServerPort()
     |       |   or
     |       |     UpnpGetServerIp6Address()
     |       |     UpnpGetServerPort6()
     |       |   or
     |       |     UpnpGetServerUlaGuaIp6Address()
     |       |     UpnpGetServerUlaGuaPort6()
     |       |__ UpnpSetWebServerRootDir()
     |       |__ if error
     |              UpnpFinish()
     |       |
     |       |__ UpnpRegisterRootDevice3()
     |       |__ if error
     |              UpnpFinish()
     |       |
     |       |__ TvDeviceStateTableInit()
     |       |__ UpnpSendAdvertisement()
     |       |__ if error
     |              UpnpFinish()
     |
     |__ pthread_create()
     |#ifdef _MSC_VER
     |__ pthread_join()
     |#else
     |__ 'Catch Ctrl-C for shutdown'
     |__ TvDeviceStop()

clang-format on
*/

namespace upnplib {
bool github_actions = std::getenv("GITHUB_ACTIONS");

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
    // SKIP on Github Actions
    if (github_actions)
        GTEST_SKIP() << "             known failing test on Github Actions";

    constexpr int argc{3};
    constexpr int argsize = sizeof(UPNPLIB_PROJECT_BINARY_DIR "/bin/tv_device");
    char arg[argc][argsize]{UPNPLIB_PROJECT_BINARY_DIR "/bin/tv_device",
                            "--webdir", SAMPLE_SOURCE_DIR "/web"};
    char* argv[argc]{arg[0], arg[1], arg[2]};

    // Test Unit; argc is valid with valid arguments.
    int ret_device_main = device_main(argc, argv);
    EXPECT_EQ(ret_device_main, UPNP_E_SUCCESS)
        << errStrEx(ret_device_main, UPNP_E_SUCCESS);

    // Stop device
    EXPECT_EQ(TvDeviceStop(), UPNP_E_SUCCESS);
}

#if 0
TEST(SampleTvDeviceTestSuite, TvDeviceStart) {
    GTEST_SKIP() << "  # With using real sockets this test has side effects on "
                    "other tests. It should be mocked.";

    constexpr char* iface{};
    constexpr unsigned short port{};
    constexpr char* desc_doc_name{};
    constexpr char web_dir_path[]{SAMPLE_SOURCE_DIR "/web"};
    constexpr int ip_mode = IP_MODE_IPV4;

    // Test Unit with default settings as far as possible
    int returned = TvDeviceStart(iface, port, desc_doc_name, web_dir_path,
                                 ip_mode, linux_print, 0);
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);

    // Stop device
    EXPECT_EQ(TvDeviceStop(), UPNP_E_SUCCESS);
}
#endif

} // namespace upnplib

//
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
