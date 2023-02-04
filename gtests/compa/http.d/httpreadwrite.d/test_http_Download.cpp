// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-02-04

#include "gmock/gmock.h"

namespace compa {
bool old_code{true}; // Managed in upnplib/gtest_main.inc
bool github_actions = std::getenv("GITHUB_ACTIONS");

TEST(EmptyTestSuite, empty_gtest) {}

} // namespace compa


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "compa/gtest_main.inc"
}
