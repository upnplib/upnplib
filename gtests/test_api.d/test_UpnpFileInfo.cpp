// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-10-06

#include "UpnpFileInfo.hpp"
#include "upnplib/gtest.hpp"
#include "upnplib/sock.hpp"
#include "gtest/gtest.h"

using ::testing::ExitedWithCode;

// Since the following struct is completely invisible outside of pupnp (because
// of some template macro magic) I have duplicated it for testing here. The
// original is located in UpnpFileInfo.cpp. Possible differences of the two
// copies in the future should be detected by the tests. --Ingo
struct s_UpnpFileInfo {
    off_t m_FileLength;
    time_t m_LastModified;
    int m_IsDirectory;
    int m_IsReadable;
    DOMString m_ContentType;
    UpnpListHead m_ExtraHeadersList;
    struct sockaddr_storage m_CtrlPtIPAddr;
    UpnpString* m_Os;
};

namespace upnplib {
bool old_code{true}; // Managed in upnplib_gtest_main.inc
bool github_actions = std::getenv("GITHUB_ACTIONS");

class CUpnpFileInfo {
    // I use this simple helper class to ensure that we always free an allocated
    // UpnpFileInfo. --Ingo
  public:
    UpnpFileInfo* info{};

    CUpnpFileInfo() { this->info = UpnpFileInfo_new(); }
    ~CUpnpFileInfo() { UpnpFileInfo_delete(this->info); }
};

class CUpnpString {
    // I use this simple helper class to ensure that we always free an allocated
    // UpnpString. --Ingo
  public:
    UpnpString* str{};

    CUpnpString() { this->str = UpnpString_new(); }
    ~CUpnpString() { UpnpString_delete(this->str); }
};

TEST(UpnpFileInfoTestSuite, new_and_verify_struct_and_delete) {
    // Test Unit
    CUpnpFileInfo f;

    EXPECT_EQ(f.info->m_FileLength, 0);
    EXPECT_EQ(f.info->m_LastModified, 0);
    EXPECT_EQ(f.info->m_IsDirectory, false);
    EXPECT_EQ(f.info->m_IsReadable, false);
    EXPECT_EQ(f.info->m_ContentType, nullptr);
    // An empty list with next == prev pointers seems to be initialized
    EXPECT_EQ(f.info->m_ExtraHeadersList.next, f.info->m_ExtraHeadersList.prev);
    EXPECT_EQ(f.info->m_CtrlPtIPAddr.ss_family, 0);
    EXPECT_EQ(UpnpString_get_Length(f.info->m_Os), 0);
    EXPECT_STREQ(UpnpString_get_String(f.info->m_Os), "");
}

TEST(UpnpFileInfoDeathTest, delete_called_with_nullptr) {
    // Here is nothing to test except that its callable without segfault.
    // Test Unit
    // This asserts NO segfault.
    ASSERT_EXIT((UpnpFileInfo_delete(nullptr), exit(0)), ExitedWithCode(0),
                ".*");
}

TEST(UpnpFileInfoDeathTest, set_get_file_length) {
    CUpnpFileInfo f;

    // Test Unit
    EXPECT_EQ(UpnpFileInfo_set_FileLength(f.info, 0), 1);
    EXPECT_EQ(UpnpFileInfo_get_FileLength(f.info), 0);

    EXPECT_EQ(UpnpFileInfo_set_FileLength(f.info, -1), 1);
    EXPECT_EQ(UpnpFileInfo_get_FileLength(f.info), -1);

    EXPECT_EQ(UpnpFileInfo_set_FileLength(f.info, 65536), 1);
    EXPECT_EQ(UpnpFileInfo_get_FileLength(f.info), 65536);

    int ret_file_length{0xAA5};
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_set_FileLength called with nullptr must "
                     "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(ret_file_length = UpnpFileInfo_set_FileLength(nullptr, 0),
                     ".*");
        EXPECT_EQ(ret_file_length, 0xAA5);

    } else {

        // This expects NO segfault.
        EXPECT_EXIT((ret_file_length = UpnpFileInfo_set_FileLength(nullptr, 0),
                     exit(0)),
                    ExitedWithCode(0), ".*");
        EXPECT_EQ(ret_file_length, 0);
    }

    EXPECT_EQ(UpnpFileInfo_get_FileLength(f.info), 65536);
}

TEST(UpnpFileInfoDeathTest, get_file_length_with_nullptr) {
    off_t file_length{(off_t)0xAA5555AA};

    // Test Unit
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_get_FileLength called with nullptr must "
                     "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(file_length = UpnpFileInfo_get_FileLength(nullptr), ".*");
        EXPECT_EQ(file_length, (off_t)0xAA5555AA);

    } else {

        // This expects NO segfault.
        EXPECT_EXIT(
            (file_length = UpnpFileInfo_get_FileLength(nullptr), exit(0)),
            ExitedWithCode(0), ".*");
        EXPECT_EQ(file_length, 0);
    }
}

TEST(UpnpFileInfoDeathTest, set_get_last_modified) {
    CUpnpFileInfo f;

    // Test Unit
    // Parameter 2 is time_t, time in seconds.
    EXPECT_EQ(UpnpFileInfo_set_LastModified(f.info, 0), 1);
    EXPECT_EQ(UpnpFileInfo_get_LastModified(f.info), 0);

    EXPECT_EQ(UpnpFileInfo_set_LastModified(f.info, -1), 1);
    EXPECT_EQ(UpnpFileInfo_get_LastModified(f.info), -1);

    EXPECT_EQ(UpnpFileInfo_set_LastModified(f.info, 75904), 1);
    EXPECT_EQ(UpnpFileInfo_get_LastModified(f.info), 75904);

    int ret_set_last_modified{0xA5A5A5};
    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " UpnpFileInfo_set_LastModified() called with nullptr must "
               "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(ret_set_last_modified =
                         UpnpFileInfo_set_LastModified(nullptr, 77),
                     ".*");
        EXPECT_EQ(ret_set_last_modified, 0xA5A5A5);

    } else {

        // This expects NO segfault.
        EXPECT_EXIT(
            (ret_set_last_modified = UpnpFileInfo_set_LastModified(nullptr, 0),
             exit(0)),
            ExitedWithCode(0), ".*");
        EXPECT_EQ(ret_set_last_modified, 0);
    }

    EXPECT_EQ(UpnpFileInfo_get_LastModified(f.info), 75904);
}

TEST(UpnpFileInfoDeathTest, get_last_modified_with_nullptr) {
    time_t last_modified{0xAAAA};
    // Test Unit
    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " UpnpFileInfo_get_LastModified() called with nullptr must "
               "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(last_modified = UpnpFileInfo_get_LastModified(nullptr),
                     ".*");
        EXPECT_EQ(last_modified, 0xAAAA);

    } else {

        // This expects NO segfault.
        EXPECT_EXIT(
            (last_modified = UpnpFileInfo_get_LastModified(nullptr), exit(0)),
            ExitedWithCode(0), ".*");
        EXPECT_EQ(last_modified, 0);
    }
}

TEST(UpnpFileInfoDeathTest, set_get_is_directory) {
    CUpnpFileInfo f;

    // Test Unit
    EXPECT_EQ(UpnpFileInfo_set_IsDirectory(f.info, true), 1);
    EXPECT_EQ(UpnpFileInfo_get_IsDirectory(f.info), 1);

    EXPECT_EQ(UpnpFileInfo_set_IsDirectory(f.info, false), 1);
    EXPECT_EQ(UpnpFileInfo_get_IsDirectory(f.info), 0);

    EXPECT_EQ(UpnpFileInfo_set_IsDirectory(f.info, true), 1);

    int is_directory{0x55AA5555};
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_set_IsDirectory() called with nullptr must "
                     "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(
            is_directory = UpnpFileInfo_set_IsDirectory(nullptr, false), ".*");
        EXPECT_EQ(is_directory, 0x55AA5555);

    } else {

        // This expects NO segfault.
        EXPECT_EXIT((UpnpFileInfo_set_IsDirectory(nullptr, false), exit(0)),
                    ExitedWithCode(0), ".*");
        EXPECT_EQ(is_directory, 0);
    }

    EXPECT_EQ(UpnpFileInfo_get_IsDirectory(f.info), 1);
}

TEST(UpnpFileInfoDeathTest, get_is_directory_with_nullptr) {
    int is_directory{0x5555};

    // Test Unit
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_get_IsDirectory() called with nullptr must "
                     "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(is_directory = UpnpFileInfo_get_IsDirectory(nullptr),
                     ".*");
        EXPECT_EQ(is_directory, 0x5555);

    } else {

        // This expects NO segfault.
        EXPECT_EXIT(
            (is_directory = UpnpFileInfo_get_IsDirectory(nullptr), exit(0)),
            ExitedWithCode(0), ".*");
        EXPECT_EQ(is_directory, 0);
    }
}

TEST(UpnpFileInfoDeathTest, set_get_is_readable) {
    CUpnpFileInfo f;

    // Test Unit
    EXPECT_EQ(UpnpFileInfo_set_IsReadable(f.info, false), 1);
    EXPECT_EQ(UpnpFileInfo_get_IsReadable(f.info), 0);

    EXPECT_EQ(UpnpFileInfo_set_IsReadable(f.info, true), 1);
    EXPECT_EQ(UpnpFileInfo_get_IsReadable(f.info), 1);

    int ret_is_readable{0xAA555A};
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_set_IsReadable() called with nullptr must "
                     "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(ret_is_readable =
                         UpnpFileInfo_set_IsReadable(nullptr, false),
                     ".*");
        EXPECT_EQ(ret_is_readable, 0xAA555A);

    } else {

        // This expects NO segfault.
        EXPECT_EXIT(
            (ret_is_readable = UpnpFileInfo_set_IsReadable(nullptr, false),
             exit(0)),
            ExitedWithCode(0), ".*");
        EXPECT_EQ(ret_is_readable, 0);
    }

    EXPECT_EQ(UpnpFileInfo_get_IsReadable(f.info), 1);
}

TEST(UpnpFileInfoDeathTest, get_is_readable_with_nullptr) {
    int is_readable{0xAA55};

    // Test Unit
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_get_IsReadable() called with nullptr must "
                     "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(is_readable = UpnpFileInfo_get_IsReadable(nullptr), ".*");
        EXPECT_EQ(is_readable, 0xAA55);

    } else {

        // This expects NO segfault.
        EXPECT_EXIT(
            (is_readable = UpnpFileInfo_get_IsReadable(nullptr), exit(0)),
            ExitedWithCode(0), ".*");
        EXPECT_EQ(is_readable, 0);
    }
}

TEST(UpnpFileInfoDeathTest, set_get_content_type) {
    CUpnpFileInfo f;

    // Test Unit
    EXPECT_EQ(UpnpFileInfo_set_ContentType(f.info, ""), 1);
    EXPECT_STREQ((DOMString)UpnpFileInfo_get_ContentType(f.info), "");
    EXPECT_STREQ((const char*)UpnpFileInfo_get_ContentType_cstr(f.info), "");

    EXPECT_EQ(UpnpFileInfo_set_ContentType(f.info, "application/octet-stream"),
              1);
    EXPECT_STREQ((DOMString)UpnpFileInfo_get_ContentType(f.info),
                 "application/octet-stream");
    EXPECT_STREQ((const char*)UpnpFileInfo_get_ContentType_cstr(f.info),
                 "application/octet-stream");

    EXPECT_EQ(UpnpFileInfo_set_ContentType(f.info, 0), 0);
    EXPECT_STREQ((DOMString)UpnpFileInfo_get_ContentType(f.info),
                 "application/octet-stream");
    EXPECT_STREQ((const char*)UpnpFileInfo_get_ContentType_cstr(f.info),
                 "application/octet-stream");

    EXPECT_EQ(UpnpFileInfo_set_ContentType(f.info, nullptr), 0);
    EXPECT_STREQ((DOMString)UpnpFileInfo_get_ContentType(f.info),
                 "application/octet-stream");
    EXPECT_STREQ((const char*)UpnpFileInfo_get_ContentType_cstr(f.info),
                 "application/octet-stream");

    const DOMString ret_get_ContentType{(char*)0x5A5A};
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_get_ContentType called with nullptr must "
                     "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(
            ret_get_ContentType = UpnpFileInfo_get_ContentType(nullptr), ".*");
        EXPECT_EQ(ret_get_ContentType, (char*)0x5A5A);

    } else {

        // This expects NO segfault.
        EXPECT_EXIT(
            (ret_get_ContentType = UpnpFileInfo_get_ContentType(nullptr),
             exit(0)),
            ExitedWithCode(0), ".*");
        EXPECT_EQ(ret_get_ContentType, "");
    }

    EXPECT_STREQ((DOMString)UpnpFileInfo_get_ContentType(f.info),
                 "application/octet-stream");
    EXPECT_STREQ((const char*)UpnpFileInfo_get_ContentType_cstr(f.info),
                 "application/octet-stream");
}

TEST(UpnpFileInfoDeathTest, set_get_extra_headers_list) {
    CUpnpFileInfo f;

    // Not only the pointer to the UpnpList header (&ExtraHeadersList) is set to
    // UpnpFileInfo, but the complete UpnpList header with next and prev
    // pointers.
    UpnpListHead ExtraHeadersList;
    UpnpListInit(&ExtraHeadersList);
    // Check list initialization
    EXPECT_EQ(ExtraHeadersList.next, &ExtraHeadersList);
    EXPECT_EQ(ExtraHeadersList.next, ExtraHeadersList.prev);

    // Test Unit
    EXPECT_EQ(UpnpFileInfo_set_ExtraHeadersList(f.info, &ExtraHeadersList), 1);
    const UpnpListHead* list_head_ptr{
        UpnpFileInfo_get_ExtraHeadersList(f.info)};

    // next and prev from the Extra Header list in UpnpFileInfo now are pointing
    // to the ExtraHeadersList above.
    EXPECT_EQ(list_head_ptr->next, &ExtraHeadersList);
    EXPECT_EQ(list_head_ptr->next, list_head_ptr->prev);

    int ret_set_ExtraHeadersList{0xA5A5};
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_set_ExtraHeadersList() called with nullptr "
                     "to list must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(ret_set_ExtraHeadersList =
                         UpnpFileInfo_set_ExtraHeadersList(f.info, nullptr),
                     ".*");
        EXPECT_EQ(ret_set_ExtraHeadersList, 0xA5A5);

    } else {

        // This expects NO segfault.
        EXPECT_EXIT((ret_set_ExtraHeadersList =
                         UpnpFileInfo_set_ExtraHeadersList(f.info, nullptr),
                     exit(0)),
                    ExitedWithCode(0), ".*");
        EXPECT_EQ(ret_set_ExtraHeadersList, 0);
    }

    EXPECT_EQ(list_head_ptr->next, &ExtraHeadersList);
    EXPECT_EQ(list_head_ptr->next, list_head_ptr->prev);
}

TEST(UpnpFileInfoDeathTest, set_extra_headers_list_with_nullptr) {
    UpnpListHead ExtraHeadersList;
    UpnpListInit(&ExtraHeadersList);

    // Test Unit
    int ret_set_ExtraHeadersList{0x55AA};
    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " UpnpFileInfo_set_ExtraHeadersList() called with nullptr must "
               "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(
            ret_set_ExtraHeadersList =
                UpnpFileInfo_set_ExtraHeadersList(nullptr, &ExtraHeadersList),
            ".*");
        EXPECT_EQ(ret_set_ExtraHeadersList, 0x55AA);

    } else {

        // This expects NO segfault.
        EXPECT_EXIT(
            (ret_set_ExtraHeadersList =
                 UpnpFileInfo_set_ExtraHeadersList(nullptr, &ExtraHeadersList),
             exit(0)),
            ExitedWithCode(0), ".*");
        EXPECT_EQ(ret_set_ExtraHeadersList, 0);
    }
}

TEST(UpnpFileInfoDeathTest, add_to_list_extra_headers_list) {
    CUpnpFileInfo f;

    // Set a valid list head to an UpnpFileInfo.
    UpnpListHead ExtraHeadersList;
    UpnpListInit(&ExtraHeadersList);
    EXPECT_EQ(UpnpFileInfo_set_ExtraHeadersList(f.info, &ExtraHeadersList), 1);

    // Create another list head to add.
    UpnpListHead ExtraHeadersListAdd;
    UpnpListInit(&ExtraHeadersListAdd);

    // Test Unit
    UpnpFileInfo_add_to_list_ExtraHeadersList(f.info, &ExtraHeadersListAdd);

    // Pointer 'next' from the Extra Header list in UpnpFileInfo now are
    // pointing to the initialized ExtraHeadersList, pointer 'prev' is pointing
    // to the added ExtraHeadersList. Due to the list management it isn't added
    // but inserted.
    const UpnpListHead* list_head_ptr{
        UpnpFileInfo_get_ExtraHeadersList(f.info)};
    EXPECT_EQ(list_head_ptr->next, &ExtraHeadersList);
    EXPECT_EQ(list_head_ptr->prev, &ExtraHeadersListAdd);

    // Add no list header
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_add_to_list_ExtraHeadersList() with adding "
                     "a nullptr must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(UpnpFileInfo_add_to_list_ExtraHeadersList(f.info, nullptr),
                     ".*");

    } else {

        // This expects NO segfault.
        EXPECT_EXIT((UpnpFileInfo_add_to_list_ExtraHeadersList(f.info, nullptr),
                     exit(0)),
                    ExitedWithCode(0), ".*");
    }

    // The current list shouldn't be modified.
    EXPECT_EQ(list_head_ptr->next, &ExtraHeadersList);
    EXPECT_EQ(list_head_ptr->prev, &ExtraHeadersListAdd);
}

TEST(UpnpFileInfoDeathTest, add_to_list_extra_headers_list_with_nullptr) {
    UpnpListHead ExtraHeadersList;
    UpnpListInit(&ExtraHeadersList);

    // Test Unit
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_add_to_list_ExtraHeadersList() called with "
                     "nullptr must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(UpnpFileInfo_add_to_list_ExtraHeadersList(
                         nullptr, &ExtraHeadersList),
                     ".*");

    } else {

        // This expects NO segfault.
        EXPECT_EXIT((UpnpFileInfo_add_to_list_ExtraHeadersList(
                         nullptr, &ExtraHeadersList),
                     exit(0)),
                    ExitedWithCode(0), ".*");
    }
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_set_get_CtrlPtIPAddr) {
    CUpnpFileInfo f;

    SockAddr sock;
    sock.addr_set("192.168.1.2", 52345);

    // Test Unit
    EXPECT_EQ(UpnpFileInfo_set_CtrlPtIPAddr(f.info, &sock.addr_ss), 1);
    const sockaddr_storage* sa_ss = UpnpFileInfo_get_CtrlPtIPAddr(f.info);

    ASSERT_EQ(sa_ss->ss_family, AF_INET);
    EXPECT_STREQ(inet_ntoa(((sockaddr_in*)sa_ss)->sin_addr), "192.168.1.2");

    // Set to an invalid UpnpFileInfo with nullptr
    int ret_set_CtrlPtIPAddr{0x55A55A};
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_set_CtrlPtIPAddr() called with a nullptr "
                     "must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(ret_set_CtrlPtIPAddr =
                         UpnpFileInfo_set_CtrlPtIPAddr(nullptr, &sock.addr_ss),
                     ".*");
        EXPECT_EQ(ret_set_CtrlPtIPAddr, 0x55A55A);

    } else {

        // This expects NO segfault.
        EXPECT_EXIT((ret_set_CtrlPtIPAddr =
                         UpnpFileInfo_set_CtrlPtIPAddr(nullptr, &sock.addr_ss),
                     exit(0)),
                    ExitedWithCode(0), ".*");
        EXPECT_EQ(ret_set_CtrlPtIPAddr, 0);
    }
}

TEST(UpnpFileInfoDeathTest,
     UpnpFileInfo_set_CtrlPtIPAddr_with_invalid_SockAddr) {
    CUpnpFileInfo f;

    // Test Unit
    int ret_set_CtrlPtIPAddr{0x55A5AA};
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_set_CtrlPtIPAddr() called with a nullptr "
                     "to a SockAddr must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(ret_set_CtrlPtIPAddr =
                         UpnpFileInfo_set_CtrlPtIPAddr(f.info, nullptr),
                     ".*");
        EXPECT_EQ(ret_set_CtrlPtIPAddr, 0x55A5AA);

    } else {

        // This expects NO segfault.
        EXPECT_EXIT((ret_set_CtrlPtIPAddr =
                         UpnpFileInfo_set_CtrlPtIPAddr(f.info, nullptr),
                     exit(0)),
                    ExitedWithCode(0), ".*");
        EXPECT_EQ(ret_set_CtrlPtIPAddr, 0);
    }
}

TEST(UpnpFileInfoTestSuite, UpnpFileInfo_get_CtrlPtIPAddr_with_nullptr) {
    const sockaddr_storage* sa_ss{};

    // Test Unit
    sa_ss = UpnpFileInfo_get_CtrlPtIPAddr(nullptr);

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_get_CtrlPtIPAddr() called with a nullptr "
                     "should return a nullptr.\n";
        EXPECT_EQ(sa_ss, (sockaddr_storage*)0x30);

    } else {

        EXPECT_EQ(sa_ss, nullptr);
    }
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_clear_CtrlPtIPAddr) {
    // Provide settings with a valid sockaddr
    CUpnpFileInfo f;
    SockAddr sock;
    sock.addr_set("192.168.1.3", 52346);
    ASSERT_EQ(UpnpFileInfo_set_CtrlPtIPAddr(f.info, &sock.addr_ss), 1);

    // Test Unit
    UpnpFileInfo_clear_CtrlPtIPAddr(f.info);

    const sockaddr_storage* sa_ss = UpnpFileInfo_get_CtrlPtIPAddr(f.info);
    ASSERT_EQ(sa_ss->ss_family, 0);
    EXPECT_STREQ(inet_ntoa(((sockaddr_in*)sa_ss)->sin_addr), "0.0.0.0");
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_clear_CtrlPtIPAddr_with_nullptr) {
    // Test Unit
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_clear_CtrlPtIPAddr() called with a nullptr "
                     "must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(UpnpFileInfo_clear_CtrlPtIPAddr(nullptr), ".*");

    } else {

        // This expects NO segfault.
        EXPECT_EXIT((UpnpFileInfo_clear_CtrlPtIPAddr(nullptr), exit(0)),
                    ExitedWithCode(0), ".*");
    }
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_set_get_Os) {
    CUpnpFileInfo f;

    // Provide an UpnpString
    CUpnpString upnp; // to use with upnp.str;
    EXPECT_TRUE(UpnpString_set_String(upnp.str, "Hello world"));

    // Test Unit
    EXPECT_TRUE(UpnpFileInfo_set_Os(f.info, upnp.str));
    const UpnpString* Os = UpnpFileInfo_get_Os(f.info);
    EXPECT_STREQ(UpnpString_get_String(Os), "Hello world");

    // Set an invalid Os
    int ret_set_Os{0x5AA555A5};
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_set_Os() called with a nullptr "
                     "to an UpnpString must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(ret_set_Os = UpnpFileInfo_set_Os(f.info, nullptr), ".*");
        EXPECT_EQ(ret_set_Os, 0x5AA555A5);

    } else {

        // This expects NO segfault.
        EXPECT_EXIT(
            (ret_set_Os = UpnpFileInfo_set_Os(f.info, nullptr), exit(0)),
            ExitedWithCode(0), ".*");
        EXPECT_EQ(ret_set_Os, 0);
    }
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_set_Os_with_nullptr) {
    // Provide an UpnpString
    CUpnpString upnp; // to use with upnp.str;
    EXPECT_TRUE(UpnpString_set_String(upnp.str, "Hello world"));

    // Test Unit
    int ret_set_Os{0x5AAA55A5};
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_set_Os() called with a nullptr "
                     "to an UpnpString must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(ret_set_Os = UpnpFileInfo_set_Os(nullptr, upnp.str), ".*");
        EXPECT_EQ(ret_set_Os, 0x5AAA55A5);

    } else {

        // This expects NO segfault.
        EXPECT_EXIT(
            (ret_set_Os = UpnpFileInfo_set_Os(nullptr, upnp.str), exit(0)),
            ExitedWithCode(0), ".*");
        EXPECT_EQ(ret_set_Os, 0);
    }
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_get_Os_with_nullptr) {
    // Test Unit
    const UpnpString* ret_get_Os{(UpnpString*)0x5AAA5AA5};
    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " UpnpFileInfo_get_Os() with a nullptr must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(ret_get_Os = UpnpFileInfo_get_Os(nullptr), ".*");
        EXPECT_EQ(ret_get_Os, (UpnpString*)0x5AAA5AA5);

    } else {

        // This expects NO segfault.
        EXPECT_EXIT((ret_get_Os = UpnpFileInfo_get_Os(nullptr), exit(0)),
                    ExitedWithCode(0), ".*");
        EXPECT_EQ(ret_get_Os, nullptr);
    }
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_get_Os_Length) {
    CUpnpFileInfo f;

    // Provide an UpnpString
    CUpnpString upnp; // to use with upnp.str;
    EXPECT_TRUE(UpnpString_set_String(upnp.str, "Hello world"));
    EXPECT_TRUE(UpnpFileInfo_set_Os(f.info, upnp.str));

    // Test Unit
    EXPECT_EQ(UpnpFileInfo_get_Os_Length(f.info), 11);

    // Use invalid file info
    size_t ret_get_Os_Length{4508046};
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_get_Os_Length() with a nullptr must not "
                     "segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(ret_get_Os_Length = UpnpFileInfo_get_Os_Length(nullptr),
                     ".*");
        EXPECT_EQ(ret_get_Os_Length, 4508046);

    } else {

        // This expects NO segfault.
        EXPECT_EXIT(
            (ret_get_Os_Length = UpnpFileInfo_get_Os_Length(nullptr), exit(0)),
            ExitedWithCode(0), ".*");
        EXPECT_EQ(ret_get_Os_Length, 0);
    }
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_get_Os_cstr) {
    CUpnpFileInfo f;

    // Provide an UpnpString
    CUpnpString upnp; // to use with upnp.str;
    EXPECT_TRUE(UpnpString_set_String(upnp.str, "Hello world"));
    EXPECT_TRUE(UpnpFileInfo_set_Os(f.info, upnp.str));

    // Test Unit
    EXPECT_STREQ(UpnpFileInfo_get_Os_cstr(f.info), "Hello world");

    // Use invalid file info
    const char* ret_get_Os_cstr{(const char*)0x4508046};
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_get_Os_cstr() with a nullptr must not "
                     "segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(ret_get_Os_cstr = UpnpFileInfo_get_Os_cstr(nullptr), ".*");
        EXPECT_EQ(ret_get_Os_cstr, (const char*)0x4508046);

    } else {

        // This expects NO segfault.
        EXPECT_EXIT(
            (ret_get_Os_cstr = UpnpFileInfo_get_Os_cstr(nullptr), exit(0)),
            ExitedWithCode(0), ".*");
        EXPECT_EQ(ret_get_Os_cstr, nullptr);
    }
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_strcpy_Os) {
    CUpnpFileInfo f;
    const char str[]{"Hello world"};

    // Test Unit
    EXPECT_TRUE(UpnpFileInfo_strcpy_Os(f.info, str));

    EXPECT_STREQ(UpnpFileInfo_get_Os_cstr(f.info), "Hello world");

    // Use invalid file info
    int ret_strcpy_Os{0x4F508046};
    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " UpnpFileInfo_strcpy_Os() with a nullptr must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(ret_strcpy_Os = UpnpFileInfo_strcpy_Os(nullptr, str),
                     ".*");
        EXPECT_EQ(ret_strcpy_Os, 0x4F508046);

    } else {

        // This expects NO segfault.
        EXPECT_EXIT(
            (ret_strcpy_Os = UpnpFileInfo_strcpy_Os(nullptr, str), exit(0)),
            ExitedWithCode(0), ".*");
        EXPECT_FALSE(ret_strcpy_Os);
    }
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_strcpy_Os_with_nullptr) {
    CUpnpFileInfo f;

    // Test Unit
    // Use invalid string
    int ret_strcpy_Os{0x4FC08046};
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_strcpy_Os() with a nullptr to the string "
                     "must not segfault on Unix.\n";

        // UpnpFileInfo_strcpy_Os() uses strdup on Unix, resp. _strdup on WIN32.
        // They have different behaviour. strdup segfaults with nullptr as
        // parameter, _strdup doesn't and returns a nullptr.
#ifdef _WIN32
        EXPECT_FALSE(UpnpFileInfo_strcpy_Os(f.info, nullptr));
#else
        // This expects segfault.
        EXPECT_DEATH(ret_strcpy_Os = UpnpFileInfo_strcpy_Os(f.info, nullptr),
                     ".*");
        EXPECT_EQ(ret_strcpy_Os, 0x4FC08046);
#endif

    } else {

        // This expects NO segfault.
        EXPECT_EXIT(
            (ret_strcpy_Os = UpnpFileInfo_strcpy_Os(f.info, nullptr), exit(0)),
            ExitedWithCode(0), ".*");
        EXPECT_FALSE(ret_strcpy_Os);
    }
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_strncpy_Os) {
    CUpnpFileInfo f;
    const char str[]{"Hello world"};

    // Test Unit
    EXPECT_TRUE(UpnpFileInfo_strncpy_Os(f.info, str, 11));
    EXPECT_STREQ(UpnpFileInfo_get_Os_cstr(f.info), "Hello world");

    EXPECT_TRUE(UpnpFileInfo_strncpy_Os(f.info, str, 10));
    EXPECT_STREQ(UpnpFileInfo_get_Os_cstr(f.info), "Hello worl");

    EXPECT_TRUE(UpnpFileInfo_strncpy_Os(f.info, str, 20));
    EXPECT_STREQ(UpnpFileInfo_get_Os_cstr(f.info), "Hello world");

    EXPECT_TRUE(UpnpFileInfo_strncpy_Os(f.info, str, 0));
    EXPECT_STREQ(UpnpFileInfo_get_Os_cstr(f.info), "");

    // size_t is unsigned int
    EXPECT_TRUE(UpnpFileInfo_strncpy_Os(f.info, str, -1));
    EXPECT_STREQ(UpnpFileInfo_get_Os_cstr(f.info), "Hello world");

    // Use invalid file info
    int ret_strncpy_Os{0x4FD08046};
    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " UpnpFileInfo_strncpy_Os() with a nullptr must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(ret_strncpy_Os = UpnpFileInfo_strncpy_Os(nullptr, str, 11),
                     ".*");
        EXPECT_EQ(ret_strncpy_Os, 0x4FD08046);

    } else {

        // This expects NO segfault.
        EXPECT_EXIT((ret_strncpy_Os = UpnpFileInfo_strncpy_Os(nullptr, str, 11),
                     exit(0)),
                    ExitedWithCode(0), ".*");
        EXPECT_FALSE(ret_strncpy_Os);
    }
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_strncpy_Os_with_nullptr) {
    CUpnpFileInfo f;

    // Test Unit
    // Use invalid string
    int ret_strncpy_Os{0x4CC08046};
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_strncpy_Os() with a nullptr to the string "
                     "must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(ret_strncpy_Os =
                         UpnpFileInfo_strncpy_Os(f.info, nullptr, 11),
                     ".*");
        EXPECT_EQ(ret_strncpy_Os, 0x4CC08046);

    } else {

        // This expects NO segfault.
        EXPECT_EXIT(
            (ret_strncpy_Os = UpnpFileInfo_strncpy_Os(f.info, nullptr, 11),
             exit(0)),
            ExitedWithCode(0), ".*");
        EXPECT_FALSE(ret_strncpy_Os);
    }
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_clear_Os) {
    CUpnpFileInfo f;
    // provide an Os string
    const char str[]{"Hello world"};
    EXPECT_TRUE(UpnpFileInfo_strcpy_Os(f.info, str));
    EXPECT_STREQ(UpnpFileInfo_get_Os_cstr(f.info), "Hello world");

    // Test Unit
    UpnpFileInfo_clear_Os(f.info);
    EXPECT_STREQ(UpnpFileInfo_get_Os_cstr(f.info), "");

    // Use invalid file info
    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " UpnpFileInfo_clear_Os() with a nullptr must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(UpnpFileInfo_clear_Os(nullptr), ".*");

    } else {

        // This expects NO segfault.
        EXPECT_EXIT((UpnpFileInfo_clear_Os(nullptr), exit(0)),
                    ExitedWithCode(0), ".*");
    }
}

} // namespace upnplib

//
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
#include "upnplib/gtest_main.inc"
}
