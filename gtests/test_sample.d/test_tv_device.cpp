// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-05-04

#include "sample/common/tv_device.cpp"

#include "gtest/gtest.h"

namespace upnplib {

TEST(SampleTvDeviceTestSuite, build_test) {
    char arg[5][23]{"build/sample/tv_device", "-i", "ens2", "-webdir",
                    "./sample/web"};
    char* argv{arg[0]};

    // argc is 0 so the function should return immediately with error
    EXPECT_EQ(device_main(0, &argv), UPNP_E_INVALID_ARGUMENT);
}

} // namespace upnplib

//
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
