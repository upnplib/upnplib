// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-10-08

#include <upnplib/general.hpp>
#include <gmock/gmock.h>


namespace upnplib {
bool old_code{true}; // Managed in upnplib/gtest_main.inc
// bool github_actions = std::getenv("GITHUB_ACTIONS");

TEST(EmptyTestSuite, simple_test) {
    TRACE("This is a TRACE output.")
    std::cout << "DEBUG! dbug = " << dbug << "\n";
    DBUG << "This is a DEBUG output.\n";
}

} // namespace upnplib


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include <upnplib/gtest_main.inc>
    return gtest_return_code; // managed in upnplib/gtest_main.inc
}
