// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-10-25

// Include source code for testing. So we have also direct access to static
// functions which need to be tested.

#include "pupnp/upnp/src/genlib/net/http/webserver.cpp"
#ifndef UPNPLIB_WITH_NATIVE_PUPNP
#include "compa/src/genlib/net/http/webserver.cpp"
#endif

#include "UpnpFileInfo.hpp"
#include "gmock/gmock.h"

//
// The web_server functions call stack
//====================================
/* // clang-format off

     web_server_init()
01)  |__ media_list_init()
     |__ membuffer_init()
     |__ glob_alias_init()
     |__ Initialize callbacks
     |__ ithread_mutex_init()

// clang-format on

01) On old code we have a vector 'gMediaTypeList' that contains members of
    document types (document_type_t), that maps a file extension to the content
    type and content subtype of a document. 'gMediaTypeList' is initialized with
    pointers into C-string 'gEncodedMediaTypes' which contains the media types.
*/

namespace compa {
bool old_code{false}; // Managed in upnplib_gtest_main.inc
bool github_actions = ::std::getenv("GITHUB_ACTIONS");

#ifdef UPNPLIB_WITH_NATIVE_PUPNP
// With new code there is no media list to initialize. We have a constant array
// there, once initialized by the compiler on startup.
TEST(WebserverTestSuite, media_list_init) {
    // Test Unit
    memset(&gMediaTypeList, 0xA5, sizeof(gMediaTypeList));
    media_list_init();

    EXPECT_STREQ(gMediaTypeList[0].file_ext, "aif");
    EXPECT_STREQ(gMediaTypeList[0].content_type, "audio");
    EXPECT_STREQ(gMediaTypeList[0].content_subtype, "aiff");

    EXPECT_STREQ(gMediaTypeList[NUM_MEDIA_TYPES - 1].file_ext, "zip");
    EXPECT_STREQ(gMediaTypeList[NUM_MEDIA_TYPES - 1].content_type,
                 "application");
    EXPECT_STREQ(gMediaTypeList[NUM_MEDIA_TYPES - 1].content_subtype, "zip");
}
#endif

TEST(WebserverTestSuite, search_extension) {
    media_list_init();

    const char* con_unused{"<unused>"};
    const char* con_type{con_unused};
    const char* con_subtype{con_unused};

    // Test Unit, first entry
    EXPECT_EQ(search_extension("aif", &con_type, &con_subtype), 0);
    EXPECT_STREQ(con_type, "audio");
    EXPECT_STREQ(con_subtype, "aiff");
    // Last entry
    EXPECT_EQ(search_extension("zip", &con_type, &con_subtype), 0);
    EXPECT_STREQ(con_type, "application");
    EXPECT_STREQ(con_subtype, "zip");

    // Looking for non existing entries.
    con_type = con_unused;
    con_subtype = con_unused;
    EXPECT_EQ(search_extension("@%§&", &con_type, &con_subtype), -1);
    EXPECT_STREQ(con_type, "<unused>");
    EXPECT_STREQ(con_subtype, "<unused>");

    EXPECT_EQ(search_extension("", &con_type, &con_subtype), -1);
    EXPECT_STREQ(con_type, "<unused>");
    EXPECT_STREQ(con_subtype, "<unused>");
}

TEST(WebserverTestSuite, get_content_type) {
    GTEST_SKIP() << "To be done";
    [[maybe_unused]] UpnpFileInfo* finfo = UpnpFileInfo_new();

    // Continue here with creating tests for the webserver module. Because of
    // extended use of UpnpFileInfo I continued first with testing module
    // UpnpFileInfo.
}

TEST(WebserverTestSuite, web_server_init_successful) {
    GTEST_SKIP() << "To be done";
}

} // namespace compa

//
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "compa/gtest_main.inc"
}
