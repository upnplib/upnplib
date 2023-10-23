// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-10-24

#include <upnplib/general.hpp>
#include <upnplib/webserver.hpp>
#include <gmock/gmock.h>

namespace utest {

using ::upnplib::Document_meta;
using ::upnplib::select_filetype;


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

} // namespace utest


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include <utest/utest_main.inc>
    return gtest_return_code; // managed in gtest_main.inc
}
