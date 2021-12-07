// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-12-07

#include "gtest/gtest.h"

#include "genlib/net/http/httpparser.cpp"

namespace upnp {

// testsuite for httpparser
//-------------------------
TEST(HttpparserTestSuite, dummy_test) {}

} // namespace upnp

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
