// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-11-06

#include "custom_gtest_tools_win.hpp"
#include "gtest/gtest.h"

namespace upnp {

TEST(ToolsTestSuite, initialize_interface_addresses) { CIfaddr4 ifaddr4; }

} // namespace upnp

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
