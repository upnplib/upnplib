// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-01-06

// Include source code for testing. So we have also direct access to static
// functions which need to be tested.
#include "pupnp/upnp/src/genlib/net/http/webserver.cpp"

#ifdef UPNPLIB_WITH_TRACE
// Disable TRACE if it is compiled in. We need to do it with instantiation of a
// class to have it disabled before other classes with TRACE in its constructor
// are instantiated. Enable on single tests with 'CEnableTrace' or for all tests
// just comment the 'disable_trace' object below.
#include <iostream>
class CDisableTrace {
  public:
    CDisableTrace() { std::clog.setstate(std::ios_base::failbit); }
    ~CDisableTrace() { std::clog.clear(); }
};
CDisableTrace disable_trace;
#endif

#include "compa/src/genlib/net/http/webserver.cpp"

// #include "UpnpFileInfo.hpp"

#include "upnplib/upnptools.hpp" // for errStrEx
#include "upnplib/gtest.hpp"

// #include "umock/stdlib.hpp"
#include "gmock/gmock.h"

using ::testing::_;
using ::testing::ExitedWithCode;

using ::upnplib::errStrEx;

#ifdef UPNPLIB_WITH_NATIVE_PUPNP
#define NS
#else
#define NS ::compa
#endif

#if false
// The web_server functions call stack
//====================================
/*
     web_server_init()
01)  |__ media_list_init()
     |__ membuffer_init()
     |__ glob_alias_init()
     |__ Initialize callbacks
     |__ ithread_mutex_init()

01) On old code we have a vector 'gMediaTypeList' that contains members of
    document types (document_type_t), that maps a file extension to the content
    type and content subtype of a document. 'gMediaTypeList' is initialized with
    pointers into C-string 'gEncodedMediaTypes' which contains the media types.
    Understood ;-) ? I simplify it on the compatible code. --Ingo

Functions to manage the pupnp global XML alias structure 'gAliasDoc'.
---------------------------------------------------------------------
These functions are re-engeneered to become part of the new XML alias structure
as methods. They are tested with the XMLaliasTestSuite. */
static UPNP_INLINE void glob_alias_init();
    // Not needed anymore. This will be done by instantiation of the structure
    // and with its constructor.
int web_server_set_alias();              // Will become method set().
static void alias_grab();                // will become method get().
static UPNP_INLINE int is_valid_alias(); // Will become method is_valid().
static void alias_release();             // Will become method release().
#endif

//
namespace compa {
bool old_code{false}; // Managed in compa/gtest_main.inc
bool github_actions = ::std::getenv("GITHUB_ACTIONS");

class CEnableTrace {
#ifdef UPNPLIB_WITH_TRACE
  public:
    CEnableTrace() { std::clog.clear(); }
    ~CEnableTrace() { std::clog.setstate(std::ios_base::failbit); }
#endif
};

//
// Little helper
// =============
class CUpnpFileInfo {
    // Use this simple helper class to ensure to always free an allocated
    // UpnpFileInfo.
  public:
    UpnpFileInfo* info{};

    CUpnpFileInfo() { this->info = UpnpFileInfo_new(); }
    ~CUpnpFileInfo() { UpnpFileInfo_delete(this->info); }
};

class CgWebMutex {
    // There are some Units with mutex locks and unlocks that will throw
    // exceptions on WIN32 if not initialized. I need this class in conjunction
    // with other helper classes to ensure that the mutex is destructed at last.
    // --Ingo
  public:
    CgWebMutex() { pthread_mutex_init(&gWebMutex, NULL); }
    ~CgWebMutex() { pthread_mutex_destroy(&gWebMutex); }
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
// With new code there is no media list to initialize. We have a constant array
// there, once initialized by the compiler on startup.
TEST(MediaListTestSuite, init) {
    if (old_code) {
        // Destroy gMediaTypeList to avoid side effects from other tests.
        memset(&gMediaTypeList, 0xAA, sizeof(gMediaTypeList));

        // Test Unit
        ::media_list_init();

        EXPECT_STREQ(gMediaTypeList[0].file_ext, "aif");
        EXPECT_STREQ(gMediaTypeList[0].content_type, "audio");
        EXPECT_STREQ(gMediaTypeList[0].content_subtype, "aiff");

        EXPECT_STREQ(gMediaTypeList[NUM_MEDIA_TYPES - 1].file_ext, "zip");
        EXPECT_STREQ(gMediaTypeList[NUM_MEDIA_TYPES - 1].content_type,
                     "application");
        EXPECT_STREQ(gMediaTypeList[NUM_MEDIA_TYPES - 1].content_subtype,
                     "zip");

    } else {

        compa::media_list_init();
    }
}

TEST(MediaListTestSuite, search_extension) {
    // Destroy gMediaTypeList to avoid side effects from other tests.
    memset(&gMediaTypeList, 0xAA, sizeof(gMediaTypeList));

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

TEST(MediaListTestSuite, get_content_type) {
    // Destroy gMediaTypeList to avoid side effects from other tests.
    memset(&gMediaTypeList, 0xAA, sizeof(gMediaTypeList));

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

TEST(MediaListDeathTest, get_content_type_with_no_filename) {
    // Destroy gMediaTypeList to avoid side effects from other tests.
    memset(&gMediaTypeList, 0xAA, sizeof(gMediaTypeList));

    CUpnpFileInfo f;

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": get_content_type called with nullptr to filename must "
                     "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(::get_content_type(nullptr, f.info), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((compa::get_content_type(nullptr, f.info), exit(0)),
                    ExitedWithCode(0), ".*");
        int ret_get_content_type{UPNP_E_INTERNAL_ERROR};
        ret_get_content_type = compa::get_content_type(nullptr, f.info);
        EXPECT_EQ(ret_get_content_type, UPNP_E_FILE_NOT_FOUND)
            << errStrEx(ret_get_content_type, UPNP_E_FILE_NOT_FOUND);
    }
}

TEST(MediaListDeathTest, get_content_type_with_no_fileinfo) {
    // Destroy gMediaTypeList to avoid side effects from other tests.
    memset(&gMediaTypeList, 0xAA, sizeof(gMediaTypeList));

    NS::media_list_init();

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": get_content_type called with nullptr to fileinfo must "
                     "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(::get_content_type("filename.txt", nullptr), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((compa::get_content_type("filename.txt", nullptr), exit(0)),
                    ExitedWithCode(0), ".*");
        int ret_get_content_type{UPNP_E_INTERNAL_ERROR};
        ret_get_content_type = compa::get_content_type("filename.txt", nullptr);
        EXPECT_EQ(ret_get_content_type, UPNP_E_INVALID_ARGUMENT)
            << errStrEx(ret_get_content_type, UPNP_E_INVALID_ARGUMENT);
    }
}

class XMLaliasFTestSuite : public ::testing::Test {
  protected:
    XMLaliasFTestSuite() {
        // There are mutexes used, so we have to initialize it.
        pthread_mutex_init(&gWebMutex, NULL);

        // There is a problem with the global structure ::gAliasDoc due to side
        // effects on other tests. So we always provide a fresh initialized and
        // unused ::gAliasDoc for every test on old_code. With compatible code
        // (!old_code) the compa::gAliasDoc structure is initialized by its
        // constructor.
        if (old_code) {
            if (gAliasDoc.doc.buf != nullptr)
                ::alias_release(&::gAliasDoc);
            memset(&::gAliasDoc, 0xAA, sizeof(::gAliasDoc));
            // Next does not allocate memory, it only nullify pointer so we do
            // not have to release it.
            ::glob_alias_init();
        }
    }

    ~XMLaliasFTestSuite() { pthread_mutex_destroy(&gWebMutex); }
};
typedef XMLaliasFTestSuite XMLaliasFDeathTest;

//
TEST(XMLaliasTestSuite, glob_alias_init_and_release) {
    // There are some mutex locks and unlocks. This must be first in scope.
    CgWebMutex global_web_mutex;

    // With the old code we may see uninitialized but plausible values on
    // preused global variables so we overwrite them with unwanted values.
    // With new code the structure is initialized with its constructor, no
    // need to call an initialization function but we do it also for
    // compatibility.
    if (old_code) {
        memset(&::gAliasDoc, 0xAA, sizeof(::gAliasDoc));
    }

    // Test Unit init.
    NS::glob_alias_init();

    NS::xml_alias_t* alias = &NS::gAliasDoc;
    // An initialized empty structure does not contain a valid alias.
    ASSERT_FALSE(NS::is_valid_alias(alias));

    // Check the empty alias.
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
}

TEST_F(XMLaliasFDeathTest, alias_release_nullptr) {
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": alias_release a nullptr must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(::alias_release(nullptr), ".*");

    } else {

        // This expects NO segfault.
        EXPECT_EXIT((compa::alias_release(nullptr), exit(0)), ExitedWithCode(0),
                    ".*");
    }
}

TEST_F(XMLaliasFDeathTest, is_valid_alias_nullptr) {
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": is_valid_alias(nullptr) must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(
            {
                int rc = ::is_valid_alias((::xml_alias_t*)nullptr);
                // Next statement is only executed if there was no segfault but
                // it's needed to suppress optimization to remove unneeded
                // return code.
                std::cout << "No segfault with rc = " << rc << "\n";
            },
            ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT(
            (compa::is_valid_alias((compa::xml_alias_t*)nullptr), exit(0)),
            ExitedWithCode(0), ".*");
        bool ret_is_valid_alias{true};
        ret_is_valid_alias =
            compa::is_valid_alias((compa::xml_alias_t*)nullptr);
        EXPECT_FALSE(ret_is_valid_alias);
    }
}

#if false
//
// Little helper
// =============
class Cxml_alias_helper {
    // I use this simple helper class to ensure that we always free an
    // initialized global XML alias structure. It needs mutexes. --Ingo
  public:
    NS::xml_alias_t* alias{&NS::gAliasDoc};

    Cxml_alias_helper() { NS::glob_alias_init(); }
    ~Cxml_alias_helper() { NS::alias_release(this->alias); }
};

//
// Testsuites
// ==========
TEST(XMLaliasTestSuite, is_valid_alias_empty_structure) {
    // SKIP on Github Actions
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";
    if (!old_code)
        GTEST_FAIL() << "Test needs rework.";

    // There are some mutex locks and unlocks. This must be first in scope.
    CgWebMutex global_web_mutex;

    // Provide an empty alias.
    Cxml_alias_helper glb;

    // Test Unit empty alias structure not to be a valid alias.
    bool ret_is_valid_alias{true};
    ret_is_valid_alias = NS::is_valid_alias((NS::xml_alias_t*)glb.alias);
    EXPECT_FALSE(ret_is_valid_alias);
}

TEST(XMLaliasTestSuite, is_valid_alias_uninitialized_structure) {
    if (old_code) {
        // Because it is impossible to test for invalid pointers it is important
        // to "nullify" the structure.
        ::xml_alias_t alias{};
        // memset(&alias, 0xA5, sizeof(xml_alias_t));
        // This "random" structure setting will report a valid alias

        // Test Unit is_valid_alias()
        EXPECT_FALSE(::is_valid_alias(&alias));

    } else {

        // Here we have a cnstructor of the struct that should do initialization
        compa::xml_alias_t alias;

        // Test Unit is_valid_alias()
        EXPECT_FALSE(compa::is_valid_alias(&alias));
    }
}

TEST(XMLaliasTestSuite, set_and_is_valid_and_release_global_alias) {
    // SKIP on Github Actions
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";
    if (!old_code)
        GTEST_FAIL() << "Test needs rework.";

    // There are some mutex locks and unlocks. This must be first in scope.
    CgWebMutex global_web_mutex;

    char alias_name[]{"is_valid_alias"};
    // The content string must be allocated on the heap because it is freed by
    // the unit.
    const char content[]{"Test for a valid alias"};
    char* alias_content = (char*)malloc(sizeof(content));
    strcpy(alias_content, content);

    // Test Unit web_server_set_alias()
    // time_t 1668095500 sec is 2022-11-10T16:51:40
    EXPECT_EQ(NS::web_server_set_alias(alias_name, alias_content,
                                       sizeof(content), 1668095500),
              0);

    EXPECT_STREQ(NS::gAliasDoc.doc.buf, "Test for a valid alias");
    EXPECT_EQ(NS::gAliasDoc.doc.length, sizeof(content));
    EXPECT_EQ(NS::gAliasDoc.doc.capacity, 23);
    EXPECT_EQ(NS::gAliasDoc.doc.size_inc, 5);

    EXPECT_STREQ(NS::gAliasDoc.name.buf, "/is_valid_alias");
    EXPECT_EQ(strlen(NS::gAliasDoc.name.buf), 15);
    // *.length is without NULL byte, sizeof with NULL byte
    EXPECT_EQ(NS::gAliasDoc.name.length, sizeof('/') + sizeof(alias_name) - 1);
    EXPECT_EQ(NS::gAliasDoc.name.capacity, 19);
    EXPECT_EQ(NS::gAliasDoc.name.size_inc, 5);

    EXPECT_EQ(*NS::gAliasDoc.ct, 1);
    EXPECT_EQ(NS::gAliasDoc.last_modified, 1668095500);

    // Test Unit is_valid_alias()
    EXPECT_TRUE(NS::is_valid_alias((NS::xml_alias_t*)&NS::gAliasDoc));

    // Test Unit alias_release()
    NS::alias_release((NS::xml_alias_t*)&NS::gAliasDoc);

    EXPECT_STREQ(NS::gAliasDoc.doc.buf, nullptr);
    EXPECT_EQ(NS::gAliasDoc.doc.length, 0);
    EXPECT_EQ(NS::gAliasDoc.doc.capacity, 0);
    EXPECT_EQ(NS::gAliasDoc.doc.size_inc, 5);

    EXPECT_STREQ(NS::gAliasDoc.name.buf, nullptr);
    EXPECT_EQ(NS::gAliasDoc.name.length, 0);
    EXPECT_EQ(NS::gAliasDoc.name.capacity, 0);
    EXPECT_EQ(NS::gAliasDoc.name.size_inc, 5);

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": Clear webserver alias with 'alias_release()' "
                     "should clear all fields of the structure.\n";
        EXPECT_NE(NS::gAliasDoc.ct, nullptr);               // This is wrong
        EXPECT_EQ(NS::gAliasDoc.last_modified, 1668095500); // This is wrong

    } else {

        EXPECT_EQ(NS::gAliasDoc.ct, nullptr);
        EXPECT_EQ(NS::gAliasDoc.last_modified, 0);
    }

    // Test Unit is_valid_alias()
    EXPECT_FALSE(NS::is_valid_alias((NS::xml_alias_t*)&NS::gAliasDoc));
}

TEST(WebserverTestSuite, set_and_remove_alias) {
    // There are some mutex locks and unlocks. This must be first in scope.
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

    EXPECT_STREQ(NS::gAliasDoc.doc.buf, nullptr);
    EXPECT_EQ(NS::gAliasDoc.doc.length, 0);
    EXPECT_EQ(NS::gAliasDoc.doc.capacity, 0);
    EXPECT_EQ(NS::gAliasDoc.doc.size_inc, 5);

    EXPECT_STREQ(NS::gAliasDoc.name.buf, nullptr);
    EXPECT_EQ(NS::gAliasDoc.name.length, 0);
    EXPECT_EQ(NS::gAliasDoc.name.capacity, 0);
    EXPECT_EQ(NS::gAliasDoc.name.size_inc, 5);

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": Clear webserver alias with 'web_server_set_alias()' "
                     "should clear all fields of the structure.\n";
        EXPECT_NE(NS::gAliasDoc.ct, nullptr);               // This is wrong
        EXPECT_EQ(NS::gAliasDoc.last_modified, 1668095500); // This is wrong

    } else {

        EXPECT_EQ(NS::gAliasDoc.ct, nullptr);
        EXPECT_EQ(NS::gAliasDoc.last_modified, 0);
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
        ASSERT_DEATH(NS::web_server_set_alias(alias_name, nullptr, 0, 0), ".*");
#endif

    } else {

        // This expects NO abort with failed assertion.
        ASSERT_EXIT(
            (NS::web_server_set_alias(alias_name, nullptr, 0, 0), exit(0)),
            ExitedWithCode(0), ".*");
        int ret_set_alias{UPNP_E_INTERNAL_ERROR};
        ret_set_alias = NS::web_server_set_alias(alias_name, nullptr, 0, 0);
        EXPECT_EQ(ret_set_alias, UPNP_E_INVALID_ARGUMENT)
            << errStrEx(ret_set_alias, UPNP_E_INVALID_ARGUMENT);
    }
}

TEST(WebserverTestSuite, set_alias_with_content_null_length) {
    // Provide needed resources.
    // There are some mutex locks and unlocks. This must be first in scope.
    CgWebMutex global_web_mutex;

    // Provide an empty alias. This initializes gAliasDoc.
    Cxml_alias_helper g_alias_doc;

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

    EXPECT_STREQ(NS::gAliasDoc.name.buf, "/valid_alias_name");
    EXPECT_EQ(strlen(NS::gAliasDoc.name.buf), 17);
    // *.length is without NULL byte, sizeof with NULL byte
    EXPECT_EQ(NS::gAliasDoc.name.length, sizeof('/') + sizeof(alias_name) - 1);
    EXPECT_EQ(NS::gAliasDoc.name.capacity, 21); // Don't know why this isn't 18.
    EXPECT_EQ(NS::gAliasDoc.name.size_inc, 5);

    EXPECT_EQ(*NS::gAliasDoc.ct, 1);
    EXPECT_EQ(NS::gAliasDoc.last_modified, 1668095500);

    EXPECT_EQ(NS::gAliasDoc.doc.length, 0);
    EXPECT_EQ(NS::gAliasDoc.doc.capacity, 0);
    EXPECT_EQ(NS::gAliasDoc.doc.size_inc, 5);

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ] " CRES << __LINE__
                  << ": web_server_set_alias() must never copy a null length "
                     "random string.\n";
        EXPECT_STREQ(NS::gAliasDoc.doc.buf, "Some unused content"); // Wrong

    } else {

        EXPECT_EQ(NS::gAliasDoc.doc.buf, nullptr);
    }
}

TEST(WebserverTestSuite, set_alias_with_zero_modified_date) {
    // Provide needed resources.
    // There are some mutex locks and unlocks. This must be first in scope.
    CgWebMutex global_web_mutex;

    // Provide an empty alias. This initializes gAliasDoc.
    Cxml_alias_helper g_alias_doc;

    const char alias_name[]{"valid_alias_name"};
    // The content string must be allocated on the heap because it is freed by
    // the unit.
    const char content[]{"Some valid content"};
    char* alias_content = (char*)malloc(sizeof(content));
    strcpy(alias_content, content);

    // Preset modifired date to 2022-11-10T16:51:41
    NS::gAliasDoc.last_modified = 1668095501;

    // Test Unit
    EXPECT_EQ(NS::web_server_set_alias(alias_name, alias_content,
                                       sizeof(alias_content), 0),
              0);

    EXPECT_EQ(NS::gAliasDoc.last_modified, 0);
}

TEST(WebserverTestSuite, set_alias_with_negative_modified_date) {
    // Provide needed resources.
    // There are some mutex locks and unlocks. This must be first in scope.
    CgWebMutex global_web_mutex;

    // Provide an empty alias. This initializes gAliasDoc.
    Cxml_alias_helper g_alias_doc;

    const char alias_name[]{"valid_alias_name"};
    // The content string must be allocated on the heap because it is freed by
    // the unit.
    const char content[]{"Some valid content"};
    char* alias_content = (char*)malloc(sizeof(content));
    strcpy(alias_content, content);

    // Preset modifired date to 2022-11-10T16:51:42
    NS::gAliasDoc.last_modified = 1668095502;

    // Test Unit
    EXPECT_EQ(NS::web_server_set_alias(alias_name, alias_content,
                                       sizeof(alias_content), -1),
              0);

    EXPECT_EQ(NS::gAliasDoc.last_modified, -1);
}

TEST(WebserverDeathTest, set_alias_three_times) {
    // This test results in a program crash with message
    // free(): double free detected in tcache 2
    // Aborted
    // It is a serious problem. To fix it this needs a complete re-engeneering
    // of handling the gAliasDoc structure.

    // SKIP on Github Actions
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // Provide needed resources.
    // There are some mutex locks and unlocks.
    CgWebMutex global_web_mutex;

    // Provide an empty alias. This initializes gAliasDoc for old code.
    NS::glob_alias_init();

    const char alias_name[]{"valid_alias_name"};
    // The content string must be allocated on the heap because it is freed by
    // the unit.
    const char content[]{"Some valid content"};
    char* alias_content = (char*)malloc(sizeof(content));
    strcpy(alias_content, content);

    // Test Unit
#ifndef __APPLE__
    // On macOS the program does not abort with this test.

    if (old_code) {
        std::cout << CRED "[ BUG      ] " CRES << __LINE__
                  << ": Set alias again to overwrite the previous setting must "
                     "not abort the program.\n";
        // This expects abort.
        ASSERT_DEATH((::web_server_set_alias(alias_name, alias_content,
                                             sizeof(alias_content), 1),
                      ::web_server_set_alias(alias_name, alias_content,
                                             sizeof(alias_content), 2),
                      ::web_server_set_alias(alias_name, alias_content,
                                             sizeof(alias_content), 3)),
                     ".*");

    } else {
#endif
        // This expects NO abort.
        ASSERT_EXIT((compa::web_server_set_alias(alias_name, alias_content,
                                                 sizeof(alias_content), 1),
                     compa::web_server_set_alias(alias_name, alias_content,
                                                 sizeof(alias_content), 2),
                     compa::web_server_set_alias(alias_name, alias_content,
                                                 sizeof(alias_content), 3),
                     exit(0)),
                    ExitedWithCode(0), ".*");
        int ret_set_alias{UPNP_E_INTERNAL_ERROR};
        ret_set_alias = compa::web_server_set_alias(alias_name, alias_content,
                                                    sizeof(alias_content), 4);
        EXPECT_EQ(ret_set_alias, UPNP_E_SUCCESS)
            << errStrEx(ret_set_alias, UPNP_E_SUCCESS);
        EXPECT_EQ(compa::gAliasDoc.last_modified, 4);
        EXPECT_EQ(*compa::gAliasDoc.ct, 1);
#ifndef __APPLE__
    }
#endif
}

TEST(WebserverTestSuite, alias_grab_valid_structure) {
    // There are some mutex locks and unlocks.
    CgWebMutex global_web_mutex;

    char alias_name[]{"is_valid_alias"};
    // No need to provide the content on the heap for this test. It isn't freed
    // there.
    const char content[]{"Test for a valid alias"};
    // char* alias_content = (char*)malloc(sizeof(content));
    // strcpy(alias_content, content);

    // Set global alias
    // time_t 1668095500 sec is 2022-11-10T16:51:40
    EXPECT_EQ(NS::web_server_set_alias(alias_name, content, sizeof(content),
                                       1668095500),
              0);
    // Provide dstination structure.
    NS::xml_alias_t gAliasDoc_dup;

    // Test Unit
    alias_grab(&gAliasDoc_dup);

    EXPECT_STREQ(gAliasDoc_dup.doc.buf, "Test for a valid alias");
    EXPECT_EQ(gAliasDoc_dup.doc.length, sizeof(content));
    EXPECT_EQ(gAliasDoc_dup.doc.capacity, 23);
    EXPECT_EQ(gAliasDoc_dup.doc.size_inc, 5);

    EXPECT_STREQ(gAliasDoc_dup.name.buf, "/is_valid_alias");
    EXPECT_EQ(strlen(gAliasDoc_dup.name.buf), 15);
    // *.length is without NULL byte, sizeof with NULL byte
    EXPECT_EQ(gAliasDoc_dup.name.length, sizeof('/') + sizeof(alias_name) - 1);
    EXPECT_EQ(gAliasDoc_dup.name.capacity, 19);
    EXPECT_EQ(gAliasDoc_dup.name.size_inc, 5);

    EXPECT_EQ(*gAliasDoc_dup.ct, 2);
    EXPECT_EQ(gAliasDoc_dup.last_modified, 1668095500);
}

TEST(WebserverDeathTest, alias_grab_empty_structure) {
    // Provide needed resources:
    // There are some mutex locks and unlocks.
    CgWebMutex global_web_mutex;

    // Provide an empty gAliasDoc.
    NS::glob_alias_init();
    // Provide dstination structure.
    NS::xml_alias_t gAliasDoc_dup;

    // Test Unit
    if (old_code) {
        std::cout << CYEL "[ FIX      ] " CRES << __LINE__
                  << ": alias_grab an empty structure should not abort the "
                     "program due to failed assertion.\n";
        // This expects abort with failed assertion.
        ASSERT_DEATH(alias_grab(&gAliasDoc_dup), ".*");

    } else {

        // This expects NO abort.
        ASSERT_EXIT((alias_grab(&gAliasDoc_dup), exit(0)), ExitedWithCode(0),
                    ".*");

        alias_grab(&gAliasDoc_dup);
        EXPECT_STREQ(gAliasDoc_dup.doc.buf, nullptr);
        EXPECT_EQ(gAliasDoc_dup.doc.length, 0);
        EXPECT_EQ(gAliasDoc_dup.doc.capacity, 0);
        EXPECT_EQ(gAliasDoc_dup.doc.size_inc, 5);

        EXPECT_STREQ(gAliasDoc_dup.name.buf, nullptr);
        EXPECT_EQ(gAliasDoc_dup.name.length, 0);
        EXPECT_EQ(gAliasDoc_dup.name.capacity, 0);
        EXPECT_EQ(gAliasDoc_dup.name.size_inc, 5);

        EXPECT_EQ(gAliasDoc_dup.last_modified, 0);
        EXPECT_EQ(gAliasDoc_dup.ct, nullptr);
    }
}

TEST(WebserverDeathTest, alias_grab_nullptr) {
    // Provide needed resources:
    // There are some mutex locks and unlocks.
    CgWebMutex global_web_mutex;

    // Provide an empty gAliasDoc.
    NS::glob_alias_init();

    // Test Unit
    if (old_code) {
        std::cout << CYEL "[ FIX      ] " CRES << __LINE__
                  << ": alias_grab(nullptr) must not segfault or abort with "
                     "failed assertion.\n";
        ASSERT_DEATH(::alias_grab(nullptr), ".*");

    } else {

        compa::alias_grab(nullptr);
    }
}
#endif

} // namespace compa

//
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "compa/gtest_main.inc"
}
