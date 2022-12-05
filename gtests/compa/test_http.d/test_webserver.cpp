// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-12-09

// Include source code for testing. So we have also direct access to static
// functions which need to be tested.
#include "pupnp/upnp/src/genlib/net/http/webserver.cpp"
#ifdef UPNPLIB_WITH_NATIVE_PUPNP
#define NS
#else
#define NS ::compa
#endif
#include "compa/src/genlib/net/http/webserver.cpp"

#include "UpnpFileInfo.hpp"

#include "upnplib/upnptools.hpp" // for errStrEx
#include "upnplib/gtest.hpp"

#include "umock/stdlib.hpp"
#include "gmock/gmock.h"

using ::testing::_;
using ::testing::ExitedWithCode;

using ::upnplib::errStrEx;

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

//
// Little helper
// =============
class CgWebMutex {
    // There are some Units with mutex locks and unlocks that will throw
    // exceptions on WIN32 if not initialized. I need this class in conjunction
    // with other helper classes to ensure that the mutex is destructed at last.
    // --Ingo
  public:
    CgWebMutex() { pthread_mutex_init(&gWebMutex, NULL); }
    ~CgWebMutex() { pthread_mutex_destroy(&gWebMutex); }
};

class CUpnpFileInfo {
    // I use this simple helper class to ensure that we always free an allocated
    // UpnpFileInfo. --Ingo
  public:
    UpnpFileInfo* info{};

    CUpnpFileInfo() { this->info = UpnpFileInfo_new(); }
    ~CUpnpFileInfo() { UpnpFileInfo_delete(this->info); }
};

class Cxml_alias {
    // I use this simple helper class to ensure that we always free an
    // initialized global XML alias structure. It needs mutexes. --Ingo
  public:
    xml_alias_t* alias{&gAliasDoc};

    Cxml_alias() { glob_alias_init(); }
    ~Cxml_alias() { compa::alias_release(this->alias); }
};

//
// Mocked system calls
//====================
class StdlibMock : public umock::StdlibInterface {
  public:
    virtual ~StdlibMock() override {}
    MOCK_METHOD(void*, malloc, (size_t size), (override));
    MOCK_METHOD(void*, calloc, (size_t nmemb, size_t size), (override));
    MOCK_METHOD(void*, realloc, (void* ptr, size_t size), (override));
    MOCK_METHOD(void, free, (void* ptr), (override));
};

//
// Testsuites
// ==========
#ifdef UPNPLIB_WITH_NATIVE_PUPNP
// With new code there is no media list to initialize. We have a constant array
// there, once initialized by the compiler on startup.
TEST(WebserverTestSuite, media_list_init) {
    // Test Unit
    memset(&gMediaTypeList, 0xA5, sizeof(gMediaTypeList));
    ::media_list_init();

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
    NS::media_list_init();

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
    CUpnpFileInfo f;
    NS::media_list_init();

    EXPECT_EQ(NS::get_content_type("tvdevicedesc.xml", f.info), 0);
    EXPECT_STREQ((DOMString)UpnpFileInfo_get_ContentType(f.info), "text/xml");

    EXPECT_EQ(NS::get_content_type("unknown_ext.???", f.info), 0);
    EXPECT_STREQ((DOMString)UpnpFileInfo_get_ContentType(f.info),
                 "application/octet-stream");

    EXPECT_EQ(NS::get_content_type("no_ext", f.info), 0);
    EXPECT_STREQ((DOMString)UpnpFileInfo_get_ContentType(f.info),
                 "application/octet-stream");

    EXPECT_EQ(NS::get_content_type("incomplete_ext.", f.info), 0);
    EXPECT_STREQ((DOMString)UpnpFileInfo_get_ContentType(f.info),
                 "application/octet-stream");

    EXPECT_EQ(NS::get_content_type(".html", f.info), 0);
    EXPECT_STREQ((DOMString)UpnpFileInfo_get_ContentType(f.info), "text/html");

    EXPECT_EQ(NS::get_content_type("double_ext.html.zip", f.info), 0);
    EXPECT_STREQ((DOMString)UpnpFileInfo_get_ContentType(f.info),
                 "application/zip");

    EXPECT_EQ(NS::get_content_type(".html.tar", f.info), 0);
    EXPECT_STREQ((DOMString)UpnpFileInfo_get_ContentType(f.info),
                 "application/tar");
}

TEST(WebserverDeathTest, get_content_type_with_no_filename) {
    CUpnpFileInfo f;

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": get_content_type called with nullptr to filename must "
                     "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::get_content_type(nullptr, f.info), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((NS::get_content_type(nullptr, f.info), exit(0)),
                    ExitedWithCode(0), ".*");
        int ret_get_content_type{UPNP_E_INTERNAL_ERROR};
        ret_get_content_type = NS::get_content_type(nullptr, f.info);
        EXPECT_EQ(ret_get_content_type, UPNP_E_FILE_NOT_FOUND)
            << errStrEx(ret_get_content_type, UPNP_E_FILE_NOT_FOUND);
    }
}

TEST(WebserverDeathTest, get_content_type_with_no_fileinfo) {
    NS::media_list_init();

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": get_content_type called with nullptr to fileinfo must "
                     "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::get_content_type("filename.txt", nullptr), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((NS::get_content_type("filename.txt", nullptr), exit(0)),
                    ExitedWithCode(0), ".*");
        int ret_get_content_type{UPNP_E_INTERNAL_ERROR};
        ret_get_content_type = NS::get_content_type("filename.txt", nullptr);
        EXPECT_EQ(ret_get_content_type, UPNP_E_INVALID_ARGUMENT)
            << errStrEx(ret_get_content_type, UPNP_E_INVALID_ARGUMENT);
    }
}

TEST(WebserverTestSuite, glob_alias_init_and_release) {
    // There are some mutex locks and unlocks. This object must be the first
    // object in scope.
    CgWebMutex global_web_mutex;

    struct xml_alias_t* alias = &gAliasDoc;

    alias->doc.buf = (char*)0xAAAA;
    alias->doc.length = 0xAAAA;
    alias->doc.capacity = 0xAAAA;
    alias->doc.size_inc = 0xAAAA;

    alias->name.buf = (char*)0xAAAA;
    alias->name.length = 0xAAAA;
    alias->name.capacity = 0xAAAA;
    alias->name.size_inc = 0xAAAA;

    alias->ct = (int*)0xAAAA;
    alias->last_modified = 0xAAAA;

    // Test Unit init
    glob_alias_init();

    EXPECT_EQ(alias->doc.buf, nullptr);
    EXPECT_EQ(alias->doc.length, 0);
    EXPECT_EQ(alias->doc.capacity, 0);
    EXPECT_EQ(alias->doc.size_inc, 5);

    EXPECT_EQ(alias->name.buf, nullptr);
    EXPECT_EQ(alias->name.length, 0);
    EXPECT_EQ(alias->name.capacity, 0);
    EXPECT_EQ(alias->name.size_inc, 5);

    EXPECT_EQ(alias->ct, nullptr);
    EXPECT_EQ(alias->last_modified, 0);

    // An empty alias structure hasn't allocated memory so there is also nothing
    // to free.
    StdlibMock mock_stdlibObj;
    umock::Stdlib stdlib_injectObj(&mock_stdlibObj);
    EXPECT_CALL(mock_stdlibObj, free(_)).Times(0);

    // Test Unit release
    NS::alias_release(alias);
}

TEST(WebserverDeathTest, alias_release_nullptr) {
    // There are some mutex locks and unlocks. This object must be the first
    // object in scope.
    CgWebMutex global_web_mutex;

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": alias_release a nullptr must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::alias_release(nullptr), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((NS::alias_release(nullptr), exit(0)), ExitedWithCode(0),
                    ".*");
    }
}

TEST(WebserverDeathTest, is_valid_alias_nullptr) {
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": is_valid_alias(nullptr) must not segfault.\n";
        // This expects segfault only with DEBUG build (seems there is an
        // assert in the used system function).
#ifndef NDEBUG
        EXPECT_DEATH(NS::is_valid_alias(nullptr), ".*");
#endif

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((NS::is_valid_alias(nullptr), exit(0)), ExitedWithCode(0),
                    ".*");
        bool ret_is_valid_alias{true};
        ret_is_valid_alias = NS::is_valid_alias(nullptr);
        EXPECT_FALSE(ret_is_valid_alias);
    }
}

TEST(WebserverTestSuite, is_valid_alias_empty_structure) {
    // There are some mutex locks and unlocks. This object must be the first
    // object in scope.
    CgWebMutex global_web_mutex;

    // Provide an empty alias.
    Cxml_alias glb;

    // Test Unit empty alias structure not to be a valid alias.
    bool ret_is_valid_alias{true};
    ret_is_valid_alias = NS::is_valid_alias(glb.alias);
    EXPECT_FALSE(ret_is_valid_alias);
}

TEST(WebserverTestSuite, is_valid_alias_uninitialized_structure) {
    // Because it is impossible to test for invalid pointers it is important to
    // always "nullify" the structure.
    xml_alias_t alias{};
    // memset(&alias, 0xA5, sizeof(xml_alias_t));
    // This "random" structure setting will report a valid alias!

    // Test Unit is_valid_alias()
    EXPECT_FALSE(NS::is_valid_alias(&alias));
}

TEST(WebserverTestSuite, set_and_is_valid_and_release_global_alias) {
    // There are some mutex locks and unlocks. This object must be the first
    // object in scope.
    CgWebMutex global_web_mutex;

    char alias_name[]{"is_valid_alias"};
    // The content string must be allocated on the heap because it is freed by
    // the unit.
    const char content[]{"Test for a valid alias"};
    char* alias_content = (char*)malloc(sizeof(content));
    strcpy(alias_content, content);

    // Test Unit web_server_set_alias()
    // time_t 1668095500 sec is 2022-11-10T16:51:40
    EXPECT_EQ(web_server_set_alias(alias_name, alias_content, sizeof(content),
                                   1668095500),
              0);

    EXPECT_STREQ(gAliasDoc.doc.buf, "Test for a valid alias");
    EXPECT_EQ(gAliasDoc.doc.length, sizeof(content));
    EXPECT_EQ(gAliasDoc.doc.capacity, 23);
    EXPECT_EQ(gAliasDoc.doc.size_inc, 5);

    EXPECT_STREQ(gAliasDoc.name.buf, "/is_valid_alias");
    EXPECT_EQ(strlen(gAliasDoc.name.buf), 15);
    // *.length is without NULL byte, sizeof with NULL byte
    EXPECT_EQ(gAliasDoc.name.length, sizeof('/') + sizeof(alias_name) - 1);
    EXPECT_EQ(gAliasDoc.name.capacity, 19);
    EXPECT_EQ(gAliasDoc.name.size_inc, 5);

    EXPECT_EQ(*gAliasDoc.ct, 1);
    EXPECT_EQ(gAliasDoc.last_modified, 1668095500);

    // Test Unit is_valid_alias()
    EXPECT_TRUE(NS::is_valid_alias(&gAliasDoc));

    // Test Unit alias_release()
    NS::alias_release(&gAliasDoc);

    EXPECT_STREQ(gAliasDoc.doc.buf, nullptr);
    EXPECT_EQ(gAliasDoc.doc.length, 0);
    EXPECT_EQ(gAliasDoc.doc.capacity, 0);
    EXPECT_EQ(gAliasDoc.doc.size_inc, 5);

    EXPECT_STREQ(gAliasDoc.name.buf, nullptr);
    EXPECT_EQ(gAliasDoc.name.length, 0);
    EXPECT_EQ(gAliasDoc.name.capacity, 0);
    EXPECT_EQ(gAliasDoc.name.size_inc, 5);

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": Clear webserver alias with 'alias_release()' "
                     "should clear all fields of the structure.\n";
        EXPECT_NE(gAliasDoc.ct, nullptr);               // This is wrong
        EXPECT_EQ(gAliasDoc.last_modified, 1668095500); // This is wrong

    } else {

        EXPECT_EQ(gAliasDoc.ct, nullptr);
        EXPECT_EQ(gAliasDoc.last_modified, 0);
    }

    // Test Unit is_valid_alias()
    EXPECT_FALSE(NS::is_valid_alias(&gAliasDoc));
}

TEST(WebserverTestSuite, set_and_remove_alias) {
    // There are some mutex locks and unlocks. This object must be the first
    // object in scope.
    CgWebMutex global_web_mutex;

    char alias_name[]{"alias_name"};
    // The content string must be allocated on the heap because it is freed by
    // the unit.
    const char content[]{"This is an alias content"};
    char* alias_content = (char*)malloc(sizeof(content));
    strcpy(alias_content, content);

    // Test Unit set_alias
    // time_t 1668095500 sec is 2022-11-10T16:51:40
    EXPECT_EQ(NS::web_server_set_alias(alias_name, alias_content,
                                       sizeof(content), 1668095500),
              0);

    // Test Unit remove alias
    // time_t 1668095500 sec is 2022-11-10T16:51:40
    EXPECT_EQ(NS::web_server_set_alias(nullptr, alias_content, sizeof(content),
                                       1668095500),
              0);

    EXPECT_STREQ(gAliasDoc.doc.buf, nullptr);
    EXPECT_EQ(gAliasDoc.doc.length, 0);
    EXPECT_EQ(gAliasDoc.doc.capacity, 0);
    EXPECT_EQ(gAliasDoc.doc.size_inc, 5);

    EXPECT_STREQ(gAliasDoc.name.buf, nullptr);
    EXPECT_EQ(gAliasDoc.name.length, 0);
    EXPECT_EQ(gAliasDoc.name.capacity, 0);
    EXPECT_EQ(gAliasDoc.name.size_inc, 5);

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": Clear webserver alias with 'web_server_set_alias()' "
                     "should clear all fields of the structure.\n";
        EXPECT_NE(gAliasDoc.ct, nullptr);               // This is wrong
        EXPECT_EQ(gAliasDoc.last_modified, 1668095500); // This is wrong

    } else {

        EXPECT_EQ(gAliasDoc.ct, nullptr);
        EXPECT_EQ(gAliasDoc.last_modified, 0);
    }
}

TEST(WebserverDeathTest, set_alias_with_nullptr_to_alias_content) {
    char alias_name[]{"alias_name"};

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": web_server_set_alias() called with nullptr to "
                     "alias_content must not abort with failed assertion.\n";
        // This expects an abort with failed assertion only with DEBUG build.
        // There is an assert used in the function.
#ifndef NDEBUG
        ASSERT_DEATH(::web_server_set_alias(alias_name, nullptr, 0, 0), ".*");
#endif

    } else {

#ifdef _WIN32
        // This expects WIN32 SEH exception due to invalid argument.
        ASSERT_DEATH(compa::web_server_set_alias(alias_name, nullptr, 0, 0),
                     ".*");
#else
        // This expects NO abort with failed assertion.
        ASSERT_EXIT(
            (compa::web_server_set_alias(alias_name, nullptr, 0, 0), exit(0)),
            ExitedWithCode(0), ".*");
        int ret_set_alias{UPNP_E_INTERNAL_ERROR};
        ret_set_alias = compa::web_server_set_alias(alias_name, nullptr, 0, 0);
        EXPECT_EQ(ret_set_alias, UPNP_E_INVALID_ARGUMENT)
            << errStrEx(ret_set_alias, UPNP_E_INVALID_ARGUMENT);
#endif
    }
}

TEST(WebserverTestSuite, set_alias_with_content_null_length) {
    // Provide needed resources.
    // There are some mutex locks and unlocks. This object must be the first
    // object in scope.
    CgWebMutex global_web_mutex;

    // Provide an empty alias. This initializes gAliasDoc.
    Cxml_alias g_alias_doc;

    char alias_name[]{"valid_alias_name"};
    // The content string must be allocated on the heap because it is freed by
    // the unit.
    char content[]{"Some unused content"};
    // Destroy terminating '\0' to test length argument.
    // content[sizeof(content)-1] = '\xAA';
    char* alias_content = (char*)malloc(sizeof(content));
    strcpy(alias_content, content);

    // Test Unit
    // time_t 1668095500 sec is 2022-11-10T16:51:40
    EXPECT_EQ(
        NS::web_server_set_alias(alias_name, alias_content, 0, 1668095500), 0);

    EXPECT_STREQ(gAliasDoc.name.buf, "/valid_alias_name");
    EXPECT_EQ(strlen(gAliasDoc.name.buf), 17);
    // *.length is without NULL byte, sizeof with NULL byte
    EXPECT_EQ(gAliasDoc.name.length, sizeof('/') + sizeof(alias_name) - 1);
    EXPECT_EQ(gAliasDoc.name.capacity, 21); // Don't know why this isn't 18.
    EXPECT_EQ(gAliasDoc.name.size_inc, 5);

    EXPECT_EQ(*gAliasDoc.ct, 1);
    EXPECT_EQ(gAliasDoc.last_modified, 1668095500);

    EXPECT_EQ(gAliasDoc.doc.length, 0);
    EXPECT_EQ(gAliasDoc.doc.capacity, 0);
    EXPECT_EQ(gAliasDoc.doc.size_inc, 5);

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": web_server_set_alias() must never copy a null length "
                     "random string.\n";
        EXPECT_STREQ(gAliasDoc.doc.buf, "Some unused content"); // This is wrong

    } else {

        EXPECT_EQ(gAliasDoc.doc.buf, nullptr);
    }
}

TEST(WebserverTestSuite, set_alias_with_zero_modified_date) {
    // Provide needed resources.
    // There are some mutex locks and unlocks. This object must be the first
    // object in scope.
    CgWebMutex global_web_mutex;

    // Provide an empty alias. This initializes gAliasDoc.
    Cxml_alias g_alias_doc;

    const char alias_name[]{"valid_alias_name"};
    // The content string must be allocated on the heap because it is freed by
    // the unit.
    const char content[]{"Some valid content"};
    char* alias_content = (char*)malloc(sizeof(content));
    strcpy(alias_content, content);

    // Preset modifired date to 2022-11-10T16:51:41
    gAliasDoc.last_modified = 1668095501;

    // Test Unit
    EXPECT_EQ(NS::web_server_set_alias(alias_name, alias_content,
                                       sizeof(alias_content), 0),
              0);

    EXPECT_EQ(gAliasDoc.last_modified, 0);
}

TEST(WebserverTestSuite, set_alias_with_negative_modified_date) {
    // Provide needed resources.
    // There are some mutex locks and unlocks. This object must be the first
    // object in scope.
    CgWebMutex global_web_mutex;

    // Provide an empty alias. This initializes gAliasDoc.
    Cxml_alias g_alias_doc;

    const char alias_name[]{"valid_alias_name"};
    // The content string must be allocated on the heap because it is freed by
    // the unit.
    const char content[]{"Some valid content"};
    char* alias_content = (char*)malloc(sizeof(content));
    strcpy(alias_content, content);

    // Preset modifired date to 2022-11-10T16:51:42
    gAliasDoc.last_modified = 1668095502;

    // Test Unit
    EXPECT_EQ(NS::web_server_set_alias(alias_name, alias_content,
                                       sizeof(alias_content), -1),
              0);

    EXPECT_EQ(gAliasDoc.last_modified, -1);
}

TEST(WebserverTestSuite, set_alias_two_times) {
    // SKIP on Github Actions
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // Provide needed resources.
    // There are some mutex locks and unlocks. This object must be the first
    // object in scope.
    CgWebMutex global_web_mutex;

    if (old_code) {
        std::cout << CRED "[ BUG      ] " CRES << __LINE__
                  << ": Using an initialized and destroyed gAliasDoc structure "
                     "must not abort the program.\n";
    }

// We cannot have the instantiation of the helper class within an if() else
// block due to its limited scope.
#ifndef UPNPLIB_WITH_NATIVE_PUPNP
    // Using the Cxml_alias results in message
    // free(): double free detected in tcache 2
    // Aborted
    // This is because is_valid_alias() does not work prolberly and the
    // helper object tries to free an already freed part of gAliasDoc. To
    // fix it this needs a complete re-engeneering of handling the gAliasDoc
    // structure.

    // Provide an empty alias. This initializes gAliasDoc.
    Cxml_alias g_alias_doc;
#endif

    const char alias_name[]{"valid_alias_name"};
    // The content string must be allocated on the heap because it is freed by
    // the unit.
    const char content[]{"Some valid content"};
    char* alias_content = (char*)malloc(sizeof(content));
    strcpy(alias_content, content);

    // Preset modifired date to 2022-11-10T16:51:43
    gAliasDoc.last_modified = 1668095503;

    // Test Unit
    EXPECT_EQ(NS::web_server_set_alias(alias_name, alias_content,
                                       sizeof(alias_content), 0),
              0);
    EXPECT_EQ(gAliasDoc.last_modified, 0);

    EXPECT_EQ(NS::web_server_set_alias(alias_name, alias_content,
                                       sizeof(alias_content), -1),
              0);
    EXPECT_EQ(gAliasDoc.last_modified, -1);
}

} // namespace compa

//
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "compa/gtest_main.inc"
}
