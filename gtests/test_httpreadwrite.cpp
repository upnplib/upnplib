// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-12-07

#include "gtest/gtest.h"

#include "genlib/net/http/httpreadwrite.cpp"

namespace upnp {

// testsuite for httpreadwrite
//----------------------------
TEST(HttpreadwriteTestSuite, dummy_test) {}

// testsuite for statcodes
//------------------------
TEST(StatcodesTestSuite, http_get_code_text) {
    // const char* code_text = ::http_get_code_text(HTTP_NOT_FOUND);
    // ::std::cout << "code_text: " << code_text << ::std::endl;
    EXPECT_STREQ(::http_get_code_text(HTTP_NOT_FOUND), "Not Found");
    EXPECT_EQ(::http_get_code_text(99), nullptr);
    EXPECT_EQ(::http_get_code_text(600), nullptr);
}

} // namespace upnp

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
