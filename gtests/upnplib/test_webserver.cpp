// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-03-08

// Include source code for testing. So we have also direct access to static
// functions which need to be tested.
#include "upnplib/src/net/http/webserver.cpp"

#include "upnplib/webserver.hpp"
#include "gmock/gmock.h"

namespace upnplib {
bool old_code{false}; // Managed in upnplib_gtest_main.inc
bool github_actions = std::getenv("GITHUB_ACTIONS");

TEST(WebserverTestSuite, select_filetype_example) {
    const Document_meta* doc_meta = select_filetype("mp3");
    if (doc_meta != nullptr) {
        std::cout << "type = " << doc_meta->type
                  << ", subtype = " << doc_meta->subtype << "\n";
    }
}

TEST(WebserverTestSuite, select_filetype) {
    const Document_meta* doc_meta{nullptr};

    // Test Unit, first entry
    doc_meta = select_filetype("aif");
    EXPECT_EQ(doc_meta->type, "audio");
    EXPECT_EQ(doc_meta->subtype, "aiff");
    // Last entry
    doc_meta = select_filetype("zip");
    EXPECT_EQ(doc_meta->type, "application");
    EXPECT_EQ(doc_meta->subtype, "zip");

    // Looking for non existing entries.
    memset(&doc_meta, 0xA5, sizeof(doc_meta));
    doc_meta = select_filetype("@%§&");
    EXPECT_EQ(doc_meta, nullptr);

    memset(&doc_meta, 0xA5, sizeof(doc_meta));
    doc_meta = select_filetype("");
    EXPECT_EQ(doc_meta, nullptr);
}

} // namespace upnplib

//
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "upnplib/gtest_main.inc"
    return gtest_return_code; // managed in upnplib/gtest_main.inc
}
