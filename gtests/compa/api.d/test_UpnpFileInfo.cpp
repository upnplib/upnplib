// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-07-17

#include <UpnpFileInfo.hpp>
#include <upnplib/sockaddr.hpp>
#include <upnplib/port.hpp>

#ifdef UPNPLIB_WITH_NATIVE_PUPNP
#define NS
#else
#define NS
#endif

#include <upnplib/gtest.hpp>
#include <gtest/gtest.h>

// This structure is completely hidden by typedef magic. It is only coppied
// here for internal testing. Usually you have to use UpnpFileInfo_new() to get
// a pointer to a structure and use getter/setter to access members. The
// original is located in pupnp/upnp/src/api/UpnpFileInfo.cpp.
// The tests need rework to respect this.
struct s_UpnpFileInfo {
    off_t m_FileLength;
    time_t m_LastModified;
    int m_IsDirectory;
    int m_IsReadable;
    DOMString m_ContentType;
    UpnpListHead m_ExtraHeadersList;
    ::sockaddr_storage m_CtrlPtIPAddr;
    UpnpString* m_Os;
};

namespace compa {
bool old_code{true}; // Managed in upnplib_gtest_main.inc
bool github_actions = std::getenv("GITHUB_ACTIONS");

using ::testing::ExitedWithCode;

class CUpnpFileInfo {
    // I use this simple helper class to ensure that we always free an allocated
    // UpnpFileInfo. --Ingo
  public:
    UpnpFileInfo* info{};

    CUpnpFileInfo() {
        TRACE2(this, " Construct CUpnpFileInfo()")
        this->info = UpnpFileInfo_new();
    }
    ~CUpnpFileInfo() {
        TRACE2(this, " Destruct CUpnpFileInfo()")
        UpnpFileInfo_delete(this->info);
    }
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
    EXPECT_EQ(NS::UpnpString_get_Length(f.info->m_Os), (size_t)0);
    EXPECT_STREQ(NS::UpnpString_get_String(f.info->m_Os), "");
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
    EXPECT_EQ(NS::UpnpFileInfo_set_FileLength(f.info, 0), 1);
    EXPECT_EQ(NS::UpnpFileInfo_get_FileLength(f.info), (off_t)0);

    EXPECT_EQ(NS::UpnpFileInfo_set_FileLength(f.info, -1), 1);
    EXPECT_EQ(NS::UpnpFileInfo_get_FileLength(f.info), (off_t)-1);

    EXPECT_EQ(NS::UpnpFileInfo_set_FileLength(f.info, 65536), 1);
    EXPECT_EQ(NS::UpnpFileInfo_get_FileLength(f.info), (off_t)65536);

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_set_FileLength called with nullptr must "
                     "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpFileInfo_set_FileLength(nullptr, 0), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((NS::UpnpFileInfo_set_FileLength(nullptr, 0), exit(0)),
                    ExitedWithCode(0), ".*");
        int ret_file_length{0xAA5}; // 2725
        ret_file_length = NS::UpnpFileInfo_set_FileLength(nullptr, 0);
        EXPECT_EQ(ret_file_length, 0);
    }

    EXPECT_EQ(NS::UpnpFileInfo_get_FileLength(f.info), (off_t)65536);
}

TEST(UpnpFileInfoDeathTest, get_file_length_with_nullptr) {
    // Test Unit
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_get_FileLength called with nullptr must "
                     "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpFileInfo_get_FileLength(nullptr), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((NS::UpnpFileInfo_get_FileLength(nullptr), exit(0)),
                    ExitedWithCode(0), ".*");
        off_t file_length{(off_t)0xAA5555AA};
        file_length = NS::UpnpFileInfo_get_FileLength(nullptr);
        EXPECT_EQ(file_length, (off_t)0);
    }
}

TEST(UpnpFileInfoDeathTest, set_get_last_modified) {
    CUpnpFileInfo f;

    // Test Unit
    // Parameter 2 is time_t, time in seconds.
    EXPECT_EQ(NS::UpnpFileInfo_set_LastModified(f.info, 0), 1);
    EXPECT_EQ(NS::UpnpFileInfo_get_LastModified(f.info), (time_t)0);

    EXPECT_EQ(NS::UpnpFileInfo_set_LastModified(f.info, -1), 1);
    EXPECT_EQ(NS::UpnpFileInfo_get_LastModified(f.info), (time_t)-1);

    EXPECT_EQ(NS::UpnpFileInfo_set_LastModified(f.info, 75904), 1);
    EXPECT_EQ(NS::UpnpFileInfo_get_LastModified(f.info), (time_t)75904);

    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " UpnpFileInfo_set_LastModified() called with nullptr must "
               "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpFileInfo_set_LastModified(nullptr, 77), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((NS::UpnpFileInfo_set_LastModified(nullptr, 0), exit(0)),
                    ExitedWithCode(0), ".*");
        int ret_set_last_modified{0xA5A5A5};
        ret_set_last_modified = NS::UpnpFileInfo_set_LastModified(nullptr, 0);
        EXPECT_EQ(ret_set_last_modified, 0);
    }

    EXPECT_EQ(NS::UpnpFileInfo_get_LastModified(f.info), (time_t)75904);
}

TEST(UpnpFileInfoDeathTest, get_last_modified_with_nullptr) {
    // Test Unit
    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " UpnpFileInfo_get_LastModified() called with nullptr must "
               "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpFileInfo_get_LastModified(nullptr), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((NS::UpnpFileInfo_get_LastModified(nullptr), exit(0)),
                    ExitedWithCode(0), ".*");
        time_t ret_last_modified{0xAAAA};
        ret_last_modified = NS::UpnpFileInfo_get_LastModified(nullptr);
        EXPECT_EQ(ret_last_modified, (time_t)0);
    }
}

TEST(UpnpFileInfoDeathTest, set_get_is_directory) {
    CUpnpFileInfo f;

    // Test Unit
    EXPECT_EQ(NS::UpnpFileInfo_set_IsDirectory(f.info, true), 1);
    EXPECT_EQ(NS::UpnpFileInfo_get_IsDirectory(f.info), 1);

    EXPECT_EQ(NS::UpnpFileInfo_set_IsDirectory(f.info, false), 1);
    EXPECT_EQ(NS::UpnpFileInfo_get_IsDirectory(f.info), 0);

    EXPECT_EQ(NS::UpnpFileInfo_set_IsDirectory(f.info, true), 1);

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_set_IsDirectory() called with nullptr must "
                     "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpFileInfo_set_IsDirectory(nullptr, false), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((NS::UpnpFileInfo_set_IsDirectory(nullptr, false), exit(0)),
                    ExitedWithCode(0), ".*");
        int ret_is_directory{0x55AA5555};
        ret_is_directory = NS::UpnpFileInfo_set_IsDirectory(nullptr, false);
        EXPECT_EQ(ret_is_directory, 0);
    }

    EXPECT_EQ(NS::UpnpFileInfo_get_IsDirectory(f.info), 1);
}

TEST(UpnpFileInfoDeathTest, get_is_directory_with_nullptr) {
    // Test Unit
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_get_IsDirectory() called with nullptr must "
                     "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpFileInfo_get_IsDirectory(nullptr), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((NS::UpnpFileInfo_get_IsDirectory(nullptr), exit(0)),
                    ExitedWithCode(0), ".*");
        int ret_is_directory{0x5555};
        ret_is_directory = NS::UpnpFileInfo_get_IsDirectory(nullptr);
        EXPECT_EQ(ret_is_directory, 0);
    }
}

TEST(UpnpFileInfoDeathTest, set_get_is_readable) {
    CUpnpFileInfo f;

    // Test Unit
    EXPECT_EQ(NS::UpnpFileInfo_set_IsReadable(f.info, false), 1);
    EXPECT_EQ(NS::UpnpFileInfo_get_IsReadable(f.info), 0);

    EXPECT_EQ(NS::UpnpFileInfo_set_IsReadable(f.info, true), 1);
    EXPECT_EQ(NS::UpnpFileInfo_get_IsReadable(f.info), 1);

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_set_IsReadable() called with nullptr must "
                     "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpFileInfo_set_IsReadable(nullptr, false), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((NS::UpnpFileInfo_set_IsReadable(nullptr, false), exit(0)),
                    ExitedWithCode(0), ".*");
        int ret_is_readable{0xAA555A};
        ret_is_readable = NS::UpnpFileInfo_set_IsReadable(nullptr, false);
        EXPECT_EQ(ret_is_readable, 0);
    }

    EXPECT_EQ(NS::UpnpFileInfo_get_IsReadable(f.info), 1);
}

TEST(UpnpFileInfoDeathTest, get_is_readable_with_nullptr) {
    // Test Unit
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_get_IsReadable() called with nullptr must "
                     "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpFileInfo_get_IsReadable(nullptr), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((NS::UpnpFileInfo_get_IsReadable(nullptr), exit(0)),
                    ExitedWithCode(0), ".*");
        int ret_is_readable{0xAA55};
        ret_is_readable = NS::UpnpFileInfo_get_IsReadable(nullptr);
        EXPECT_EQ(ret_is_readable, 0);
    }
}

TEST(UpnpFileInfoDeathTest, set_get_content_type) {
    CUpnpFileInfo f;

    // Test Unit
    EXPECT_EQ(NS::UpnpFileInfo_set_ContentType(f.info, ""), 1);
    EXPECT_STREQ((DOMString)NS::UpnpFileInfo_get_ContentType(f.info), "");
    EXPECT_STREQ((const char*)NS::UpnpFileInfo_get_ContentType_cstr(f.info),
                 "");

    EXPECT_EQ(
        NS::UpnpFileInfo_set_ContentType(f.info, "application/octet-stream"),
        1);
    EXPECT_STREQ((DOMString)NS::UpnpFileInfo_get_ContentType(f.info),
                 "application/octet-stream");
    EXPECT_STREQ((const char*)NS::UpnpFileInfo_get_ContentType_cstr(f.info),
                 "application/octet-stream");

    EXPECT_EQ(NS::UpnpFileInfo_set_ContentType(f.info, 0), 0);
    EXPECT_STREQ((DOMString)NS::UpnpFileInfo_get_ContentType(f.info),
                 "application/octet-stream");
    EXPECT_STREQ((const char*)NS::UpnpFileInfo_get_ContentType_cstr(f.info),
                 "application/octet-stream");

    EXPECT_EQ(NS::UpnpFileInfo_set_ContentType(f.info, nullptr), 0);
    EXPECT_STREQ((DOMString)NS::UpnpFileInfo_get_ContentType(f.info),
                 "application/octet-stream");
    EXPECT_STREQ((const char*)NS::UpnpFileInfo_get_ContentType_cstr(f.info),
                 "application/octet-stream");

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_get_ContentType called with nullptr must "
                     "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpFileInfo_get_ContentType(nullptr), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((NS::UpnpFileInfo_get_ContentType(nullptr), exit(0)),
                    ExitedWithCode(0), ".*");
        const DOMString ret_get_ContentType{(char*)0x5A5A};
        ret_get_ContentType = NS::UpnpFileInfo_get_ContentType(nullptr);
        EXPECT_STREQ(ret_get_ContentType, nullptr);
    }

    EXPECT_STREQ((DOMString)NS::UpnpFileInfo_get_ContentType(f.info),
                 "application/octet-stream");
}

TEST(UpnpFileInfoDeathTest, set_ContentType_with_nullptr) {
    // Test Unit
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_set_ContentType() called with nullptr must "
                     "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpFileInfo_set_ContentType(nullptr, "segfault"),
                     ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT(
            (NS::UpnpFileInfo_set_ContentType(nullptr, "segfault"), exit(0)),
            ExitedWithCode(0), ".*");
        int ret_set_ContentType{0xAA55A};
        ret_set_ContentType =
            NS::UpnpFileInfo_set_ContentType(nullptr, "segfault");
        EXPECT_EQ(ret_set_ContentType, 0);
    }
}

TEST(UpnpFileInfoDeathTest, get_ContentType_cstr_with_nullptr) {
    // Test Unit
    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " UpnpFileInfo_get_ContentType_cstr() called with nullptr must "
               "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpFileInfo_get_ContentType_cstr(nullptr), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((NS::UpnpFileInfo_get_ContentType_cstr(nullptr), exit(0)),
                    ExitedWithCode(0), ".*");
        const char* ret_get_ContentType_cstr{""};
        ret_get_ContentType_cstr =
            NS::UpnpFileInfo_get_ContentType_cstr(nullptr);
        EXPECT_EQ(ret_get_ContentType_cstr, nullptr);
    }
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
    EXPECT_EQ(NS::UpnpFileInfo_set_ExtraHeadersList(f.info, &ExtraHeadersList),
              1);
    const UpnpListHead* list_head_ptr{
        NS::UpnpFileInfo_get_ExtraHeadersList(f.info)};

    // next and prev from the Extra Header list in UpnpFileInfo now are pointing
    // to the ExtraHeadersList above.
    EXPECT_EQ(list_head_ptr->next, &ExtraHeadersList);
    EXPECT_EQ(list_head_ptr->next, list_head_ptr->prev);

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_set_ExtraHeadersList() called with nullptr "
                     "to list must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpFileInfo_set_ExtraHeadersList(f.info, nullptr),
                     ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT(
            (NS::UpnpFileInfo_set_ExtraHeadersList(f.info, nullptr), exit(0)),
            ExitedWithCode(0), ".*");
        int ret_set_ExtraHeadersList{0xA5A5};
        ret_set_ExtraHeadersList =
            NS::UpnpFileInfo_set_ExtraHeadersList(f.info, nullptr);
        EXPECT_EQ(ret_set_ExtraHeadersList, 0);
    }

    EXPECT_EQ(list_head_ptr->next, &ExtraHeadersList);
    EXPECT_EQ(list_head_ptr->next, list_head_ptr->prev);
}

TEST(UpnpFileInfoDeathTest, set_extra_headers_list_with_nullptr) {
    UpnpListHead ExtraHeadersList;
    UpnpListInit(&ExtraHeadersList);

    // Test Unit
    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " UpnpFileInfo_set_ExtraHeadersList() called with nullptr must "
               "not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(
            NS::UpnpFileInfo_set_ExtraHeadersList(nullptr, &ExtraHeadersList),
            ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT(
            (NS::UpnpFileInfo_set_ExtraHeadersList(nullptr, &ExtraHeadersList),
             exit(0)),
            ExitedWithCode(0), ".*");
        int ret_set_ExtraHeadersList{0x55AA};
        ret_set_ExtraHeadersList =
            NS::UpnpFileInfo_set_ExtraHeadersList(nullptr, &ExtraHeadersList);
        EXPECT_EQ(ret_set_ExtraHeadersList, 0);
    }
}

TEST(UpnpFileInfoDeathTest, add_to_list_extra_headers_list) {
    CUpnpFileInfo f;

    // Set a valid list head to an UpnpFileInfo.
    UpnpListHead ExtraHeadersList;
    UpnpListInit(&ExtraHeadersList);
    EXPECT_EQ(NS::UpnpFileInfo_set_ExtraHeadersList(f.info, &ExtraHeadersList),
              1);

    // Create another list head to add.
    UpnpListHead ExtraHeadersListAdd;
    UpnpListInit(&ExtraHeadersListAdd);

    // Test Unit
    NS::UpnpFileInfo_add_to_list_ExtraHeadersList(f.info, &ExtraHeadersListAdd);

    // Pointer 'next' from the Extra Header list in UpnpFileInfo now are
    // pointing to the initialized ExtraHeadersList, pointer 'prev' is pointing
    // to the added ExtraHeadersList. Due to the list management it isn't added
    // but inserted.
    const UpnpListHead* list_head_ptr{
        NS::UpnpFileInfo_get_ExtraHeadersList(f.info)};
    EXPECT_EQ(list_head_ptr->next, &ExtraHeadersList);
    EXPECT_EQ(list_head_ptr->prev, &ExtraHeadersListAdd);

    // Add no list header
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_add_to_list_ExtraHeadersList() with adding "
                     "a nullptr must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(
            NS::UpnpFileInfo_add_to_list_ExtraHeadersList(f.info, nullptr),
            ".*");

    } else {

        // This expects NO segfault.
        EXPECT_EXIT(
            (NS::UpnpFileInfo_add_to_list_ExtraHeadersList(f.info, nullptr),
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
        EXPECT_DEATH(NS::UpnpFileInfo_add_to_list_ExtraHeadersList(
                         nullptr, &ExtraHeadersList),
                     ".*");

    } else {

        // This expects NO segfault.
        EXPECT_EXIT((NS::UpnpFileInfo_add_to_list_ExtraHeadersList(
                         nullptr, &ExtraHeadersList),
                     exit(0)),
                    ExitedWithCode(0), ".*");
    }
}

// This test fails only on MacOS due to pointer problems. I don't know why the
// clang compiler has problems to give the correct pointer from
// ::upnplib::SSockaddr_storage to the function UpnpFileInfo_set_CtrlPtIPAddr()
// as argument. This test is the reason to reassign the structure
// ::upnplib::SSockaddr_storage. --Ingo
TEST(UpnpFileInfoTestSuite, UpnpFileInfo_set_get_CtrlPtIPAddr) {
    UpnpFileInfo* info = UpnpFileInfo_new();
    info->m_CtrlPtIPAddr.ss_family = AF_INET6;

    ::upnplib::SSockaddr_storage saddr{};
    saddr = "192.168.11.12:52345";
    EXPECT_EQ(saddr.ss.ss_family, AF_INET);
    EXPECT_EQ(info->m_CtrlPtIPAddr.ss_family, AF_INET6);

    // Test Unit
    EXPECT_EQ(UpnpFileInfo_set_CtrlPtIPAddr(info, &saddr.ss), 1);

    EXPECT_EQ(saddr.ss.ss_family, AF_INET);
    EXPECT_EQ(info->m_CtrlPtIPAddr.ss_family, AF_INET);
    char addr4buf[16];
    EXPECT_STREQ(inet_ntop(AF_INET,
                           &((sockaddr_in*)&info->m_CtrlPtIPAddr)->sin_addr,
                           addr4buf, sizeof(addr4buf)),
                 "192.168.11.12");
    EXPECT_EQ(ntohs(((sockaddr_in*)&info->m_CtrlPtIPAddr)->sin_port), 52345);

    UpnpFileInfo_delete(info);
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_set_get_CtrlPtIPAddr) {
    CUpnpFileInfo f;

    f.info->m_CtrlPtIPAddr.ss_family = AF_INET6;
    ::sockaddr_storage ss{};
    ss.ss_family = AF_INET;
    sockaddr_in* const sin = (sockaddr_in*)&ss;
    ASSERT_EQ(inet_pton(AF_INET, "192.168.1.2", &sin->sin_addr), 1);
    sin->sin_port = htons(52345);

    // Test Unit
    EXPECT_EQ(UpnpFileInfo_set_CtrlPtIPAddr(f.info, &ss), 1);
    EXPECT_EQ(f.info->m_CtrlPtIPAddr.ss_family, AF_INET);

    const ::sockaddr_storage* sa_ss = NS::UpnpFileInfo_get_CtrlPtIPAddr(f.info);

    ASSERT_EQ(sa_ss->ss_family, AF_INET);
    char addr4buf[16];
    EXPECT_STREQ(inet_ntop(AF_INET, &((sockaddr_in*)sa_ss)->sin_addr, addr4buf,
                           sizeof(addr4buf)),
                 "192.168.1.2");

    // Set to an invalid UpnpFileInfo with nullptr
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_set_CtrlPtIPAddr() called with a nullptr "
                     "must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpFileInfo_set_CtrlPtIPAddr(nullptr, &ss), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((NS::UpnpFileInfo_set_CtrlPtIPAddr(nullptr, &ss), exit(0)),
                    ExitedWithCode(0), ".*");
        int ret_set_CtrlPtIPAddr{0x55A55A};
        ret_set_CtrlPtIPAddr = NS::UpnpFileInfo_set_CtrlPtIPAddr(nullptr, &ss);
        EXPECT_EQ(ret_set_CtrlPtIPAddr, 0);
    }
}

TEST(UpnpFileInfoDeathTest,
     UpnpFileInfo_set_CtrlPtIPAddr_with_invalid_SockAddr) {
    CUpnpFileInfo f;

    // Test Unit
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_set_CtrlPtIPAddr() called with a nullptr "
                     "to a SockAddr must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpFileInfo_set_CtrlPtIPAddr(f.info, nullptr), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT(
            (NS::UpnpFileInfo_set_CtrlPtIPAddr(f.info, nullptr), exit(0)),
            ExitedWithCode(0), ".*");
        int ret_set_CtrlPtIPAddr{0x55A5AA};
        ret_set_CtrlPtIPAddr =
            NS::UpnpFileInfo_set_CtrlPtIPAddr(f.info, nullptr);
        EXPECT_EQ(ret_set_CtrlPtIPAddr, 0);
    }
}

TEST(UpnpFileInfoTestSuite, UpnpFileInfo_get_CtrlPtIPAddr_with_nullptr) {
    const ::sockaddr_storage* saddr{};

    // Test Unit
    saddr = NS::UpnpFileInfo_get_CtrlPtIPAddr(nullptr);

    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_get_CtrlPtIPAddr() called with a nullptr "
                     "should return a nullptr.\n";
        EXPECT_NE(saddr, nullptr);

    } else {

        EXPECT_EQ(saddr, nullptr);
    }
}

TEST(UpnpFileInfoTestSuite, UpnpFileInfo_clear_CtrlPtIPAddr) {
    // Provide settings with a valid sockaddr
    CUpnpFileInfo f;
    char addr4buf[16];

    ::sockaddr_storage ss{};
    ss.ss_family = AF_INET;
    sockaddr_in* const sin = (sockaddr_in*)&ss;
    ASSERT_EQ(inet_pton(AF_INET, "192.168.1.3", &sin->sin_addr), 1);
    sin->sin_port = htons(52346);
    ASSERT_EQ(NS::UpnpFileInfo_set_CtrlPtIPAddr(f.info, &ss), 1);

    // Test Unit
    NS::UpnpFileInfo_clear_CtrlPtIPAddr(f.info);

    const ::sockaddr_storage* saddr = NS::UpnpFileInfo_get_CtrlPtIPAddr(f.info);
    ASSERT_EQ(saddr->ss_family, 0);
    EXPECT_STREQ(inet_ntop(AF_INET, &((sockaddr_in*)saddr)->sin_addr, addr4buf,
                           sizeof(addr4buf)),
                 "0.0.0.0");
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_clear_CtrlPtIPAddr_with_nullptr) {
    // Test Unit
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_clear_CtrlPtIPAddr() called with a nullptr "
                     "must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(
            NS::UpnpFileInfo_clear_CtrlPtIPAddr((UpnpFileInfo*)nullptr), ".*");

    } else {

        // This expects NO segfault.
        EXPECT_EXIT(
            (NS::UpnpFileInfo_clear_CtrlPtIPAddr((UpnpFileInfo*)nullptr),
             exit(0)),
            ExitedWithCode(0), ".*");
    }
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_set_get_Os) {
    CUpnpFileInfo f;

    // Provide an UpnpString
    CUpnpString upnp; // to use with upnp.str;
    EXPECT_TRUE(NS::UpnpString_set_String(upnp.str, "Hello world"));

    // Test Unit
    EXPECT_TRUE(NS::UpnpFileInfo_set_Os(f.info, upnp.str));
    const UpnpString* Os = NS::UpnpFileInfo_get_Os(f.info);
    EXPECT_STREQ(NS::UpnpString_get_String(Os), "Hello world");

    // Set an invalid Os
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_set_Os() called with a nullptr "
                     "to an UpnpString must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpFileInfo_set_Os(f.info, nullptr), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((NS::UpnpFileInfo_set_Os(f.info, nullptr), exit(0)),
                    ExitedWithCode(0), ".*");
        int ret_set_Os{0x5AA555A5};
        ret_set_Os = NS::UpnpFileInfo_set_Os(f.info, nullptr);
        EXPECT_EQ(ret_set_Os, 0);
    }
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_set_Os_with_nullptr) {
    // Provide an UpnpString
    CUpnpString upnp; // to use with upnp.str;
    EXPECT_TRUE(NS::UpnpString_set_String(upnp.str, "Hello world"));

    // Test Unit
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_set_Os() called with a nullptr "
                     "to an UpnpString must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpFileInfo_set_Os(nullptr, upnp.str), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((NS::UpnpFileInfo_set_Os(nullptr, upnp.str), exit(0)),
                    ExitedWithCode(0), ".*");
        int ret_set_Os{0x5AAA55A5};
        ret_set_Os = NS::UpnpFileInfo_set_Os(nullptr, upnp.str);
        EXPECT_EQ(ret_set_Os, 0);
    }
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_get_Os_with_nullptr) {
    // Test Unit
    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " UpnpFileInfo_get_Os() with a nullptr must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpFileInfo_get_Os((UpnpFileInfo*)nullptr), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((NS::UpnpFileInfo_get_Os((UpnpFileInfo*)nullptr), exit(0)),
                    ExitedWithCode(0), ".*");
        const UpnpString* ret_get_Os{(UpnpString*)0x5AAA5AA5};
        ret_get_Os = NS::UpnpFileInfo_get_Os((UpnpFileInfo*)nullptr);
        EXPECT_EQ(ret_get_Os, nullptr);
    }
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_get_Os_Length) {
    CUpnpFileInfo f;

    // Provide an UpnpString
    CUpnpString upnp; // to use with upnp.str;
    EXPECT_TRUE(NS::UpnpString_set_String(upnp.str, "Hello world"));
    EXPECT_TRUE(NS::UpnpFileInfo_set_Os(f.info, upnp.str));

    // Test Unit
    EXPECT_EQ(NS::UpnpFileInfo_get_Os_Length(f.info), (size_t)11);

    // Use invalid file info
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_get_Os_Length() with a nullptr must not "
                     "segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpFileInfo_get_Os_Length((UpnpFileInfo*)nullptr),
                     ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT(
            (NS::UpnpFileInfo_get_Os_Length((UpnpFileInfo*)nullptr), exit(0)),
            ExitedWithCode(0), ".*");
        size_t ret_get_Os_Length{4508046};
        ret_get_Os_Length =
            NS::UpnpFileInfo_get_Os_Length((UpnpFileInfo*)nullptr);
        EXPECT_EQ(ret_get_Os_Length, (size_t)0);
    }
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_get_Os_cstr) {
    CUpnpFileInfo f;

    // Provide an UpnpString
    CUpnpString upnp; // to use with upnp.str;
    EXPECT_TRUE(NS::UpnpString_set_String(upnp.str, "Hello world"));
    EXPECT_TRUE(NS::UpnpFileInfo_set_Os(f.info, upnp.str));

    // Test Unit
    EXPECT_STREQ(NS::UpnpFileInfo_get_Os_cstr(f.info), "Hello world");

    // Use invalid file info
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_get_Os_cstr() with a nullptr must not "
                     "segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpFileInfo_get_Os_cstr((UpnpFileInfo*)nullptr),
                     ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT(
            (NS::UpnpFileInfo_get_Os_cstr((UpnpFileInfo*)nullptr), exit(0)),
            ExitedWithCode(0), ".*");
        const char* ret_get_Os_cstr{(const char*)0x4508046};
        ret_get_Os_cstr = NS::UpnpFileInfo_get_Os_cstr((UpnpFileInfo*)nullptr);
        EXPECT_EQ(ret_get_Os_cstr, nullptr);
    }
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_strcpy_Os) {
    CUpnpFileInfo f;
    const char str[]{"Hello world"};

    // Test Unit
    EXPECT_TRUE(NS::UpnpFileInfo_strcpy_Os(f.info, str));

    EXPECT_STREQ(NS::UpnpFileInfo_get_Os_cstr(f.info), "Hello world");

    // Use invalid file info
    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " UpnpFileInfo_strcpy_Os() with a nullptr must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpFileInfo_strcpy_Os((UpnpFileInfo*)nullptr, str),
                     ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT(
            (NS::UpnpFileInfo_strcpy_Os((UpnpFileInfo*)nullptr, str), exit(0)),
            ExitedWithCode(0), ".*");
        int ret_strcpy_Os{0x4F508046};
        ret_strcpy_Os = NS::UpnpFileInfo_strcpy_Os((UpnpFileInfo*)nullptr, str);
        EXPECT_FALSE(ret_strcpy_Os);
    }
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_strcpy_Os_with_nullptr) {
    CUpnpFileInfo f;

    // Test Unit
    // Use invalid string
    if (old_code) {
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " UpnpFileInfo_strcpy_Os() with a nullptr to the string "
                     "must not segfault on Unix.\n";

        // UpnpFileInfo_strcpy_Os() uses strdup on Unix, resp. _strdup on WIN32.
        // They have different behaviour. strdup segfaults with nullptr as
        // parameter, _strdup doesn't and returns a nullptr.
#ifdef _WIN32
        EXPECT_FALSE(NS::UpnpFileInfo_strcpy_Os(f.info, nullptr));
#else
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpFileInfo_strcpy_Os(f.info, nullptr), ".*");
#endif

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((NS::UpnpFileInfo_strcpy_Os(f.info, nullptr), exit(0)),
                    ExitedWithCode(0), ".*");
        int ret_strcpy_Os{0x4FC08046};
        ret_strcpy_Os = NS::UpnpFileInfo_strcpy_Os(f.info, nullptr);
        EXPECT_FALSE(ret_strcpy_Os);
    }
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_strncpy_Os) {
    CUpnpFileInfo f;
    const char str[]{"Hello world"};

    // Test Unit
    EXPECT_TRUE(NS::UpnpFileInfo_strncpy_Os(f.info, str, 11));
    EXPECT_STREQ(NS::UpnpFileInfo_get_Os_cstr(f.info), "Hello world");

    EXPECT_TRUE(NS::UpnpFileInfo_strncpy_Os(f.info, str, 10));
    EXPECT_STREQ(NS::UpnpFileInfo_get_Os_cstr(f.info), "Hello worl");

    EXPECT_TRUE(NS::UpnpFileInfo_strncpy_Os(f.info, str, 20));
    EXPECT_STREQ(NS::UpnpFileInfo_get_Os_cstr(f.info), "Hello world");

    EXPECT_TRUE(NS::UpnpFileInfo_strncpy_Os(f.info, str, 0));
    EXPECT_STREQ(NS::UpnpFileInfo_get_Os_cstr(f.info), "");

    // (size_t)-1 = 18446744073709551615
    EXPECT_TRUE(NS::UpnpFileInfo_strncpy_Os(f.info, str, (size_t)-1));
    EXPECT_STREQ(NS::UpnpFileInfo_get_Os_cstr(f.info), "Hello world");

    // Use invalid file info
    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " UpnpFileInfo_strncpy_Os() with a nullptr must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(
            NS::UpnpFileInfo_strncpy_Os((UpnpFileInfo*)nullptr, str, 11), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT(
            (NS::UpnpFileInfo_strncpy_Os((UpnpFileInfo*)nullptr, str, 11),
             exit(0)),
            ExitedWithCode(0), ".*");
        int ret_strncpy_Os{0x4FD08046};
        ret_strncpy_Os =
            NS::UpnpFileInfo_strncpy_Os((UpnpFileInfo*)nullptr, str, 11);
        EXPECT_FALSE(ret_strncpy_Os);
    }
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_strncpy_Os_with_nullptr) {
    CUpnpFileInfo f;

    // Test Unit
    // Use invalid string
    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " NS::UpnpFileInfo_strncpy_Os() with a nullptr to the string "
               "must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(
            NS::UpnpFileInfo_strncpy_Os(f.info, (const char*)nullptr, 11),
            ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT(
            (NS::UpnpFileInfo_strncpy_Os(f.info, (const char*)nullptr, 11),
             exit(0)),
            ExitedWithCode(0), ".*");
        int ret_strncpy_Os{0x4CC08046};
        ret_strncpy_Os =
            NS::UpnpFileInfo_strncpy_Os(f.info, (const char*)nullptr, 11);
        EXPECT_FALSE(ret_strncpy_Os);
    }
}

TEST(UpnpFileInfoDeathTest, UpnpFileInfo_clear_Os) {
    CUpnpFileInfo f;
    // provide an Os string
    const char str[]{"Hello world"};
    EXPECT_TRUE(NS::UpnpFileInfo_strcpy_Os(f.info, str));
    EXPECT_STREQ(NS::UpnpFileInfo_get_Os_cstr(f.info), "Hello world");

    // Test Unit
    NS::UpnpFileInfo_clear_Os(f.info);
    EXPECT_STREQ(NS::UpnpFileInfo_get_Os_cstr(f.info), "");

    // Use invalid file info
    if (old_code) {
        std::cout
            << CYEL "[ BUGFIX   ]" CRES
            << " UpnpFileInfo_clear_Os() with a nullptr must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(NS::UpnpFileInfo_clear_Os((UpnpFileInfo*)nullptr), ".*");

    } else {

        // This expects NO segfault.
        EXPECT_EXIT(
            (NS::UpnpFileInfo_clear_Os((UpnpFileInfo*)nullptr), exit(0)),
            ExitedWithCode(0), ".*");
    }
}

} // namespace compa


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
#include "compa/gtest_main.inc"
    return gtest_return_code; // managed in compa/gtest_main.inc
}
