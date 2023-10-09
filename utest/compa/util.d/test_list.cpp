// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-10-09

#include <list.hpp>

#include <upnplib/general.hpp>
#include <upnplib/gtest.hpp>
#include <gtest/gtest.h>

using ::testing::ExitedWithCode;

namespace utest {
bool old_code{false}; // Managed in gtest_main.inc

// Interface for the list module
// =============================
// clang-format off

class Ilist {
  public:
    virtual ~Ilist() {}

    virtual void UpnpListInit(
        UpnpListHead* list) = 0;
    virtual UpnpListIter UpnpListBegin(
        UpnpListHead* list) = 0;
    virtual UpnpListIter UpnpListEnd(
        UpnpListHead* list) = 0;
    virtual UpnpListIter UpnpListNext(
        UpnpListHead* list, UpnpListIter pos) = 0;
    virtual UpnpListIter UpnpListInsert(
        UpnpListHead* list, UpnpListIter pos, UpnpListHead* elt) = 0;
    virtual UpnpListIter UpnpListErase(
        UpnpListHead* list, UpnpListIter pos) = 0;
};

class Clist : Ilist {
  public:
    virtual ~Clist() override {}

    void UpnpListInit(UpnpListHead* list) override {
        return ::UpnpListInit(list); }
    UpnpListIter UpnpListBegin(UpnpListHead* list) override {
        return ::UpnpListBegin(list); }
    UpnpListIter UpnpListEnd(UpnpListHead* list) override {
        return ::UpnpListEnd(list); }
    UpnpListIter UpnpListNext(UpnpListHead* list, UpnpListIter pos) override {
        return ::UpnpListNext(list, pos); }
    UpnpListIter UpnpListInsert(UpnpListHead* list, UpnpListIter pos, UpnpListHead* elt) override {
        return ::UpnpListInsert(list, pos, elt); }
    UpnpListIter UpnpListErase(UpnpListHead* list, UpnpListIter pos) override {
        return ::UpnpListErase(list, pos); }
};
// clang-format on

//
// testsuite for the list module
//==============================
#if false
TEST(ListTestSuite, UpnpList_show_entries)
// This "test" is only for humans to get an idea what's going on. If you need
// it, set '#if true' only temporary. It is not intended to be permanent part of
// the test suite because it doesn't really test things.
{
    UpnpListHead list{};

    // Initialize list
    Clist listObj{};
    listObj.UpnpListInit(&list);
    ::std::cout << "DEBUG: list ref       = " << &list << ::std::endl;
    ::std::cout << "DEBUG: list begin     = " << listObj.UpnpListBegin(&list)
                << ::std::endl;
    ::std::cout << "DEBUG: list end       = " << listObj.UpnpListEnd(&list)
                << ::std::endl;
    ::std::cout << "DEBUG: list next      = " << list.next << ::std::endl;
    ::std::cout << "DEBUG: list prev      = " << list.prev << ::std::endl;

    // Insert element before end of list
    UpnpListHead list_inserted{};
    UpnpListIter list_end_ptr = listObj.UpnpListEnd(&list);
    UpnpListIter list_inserted_ptr =
        listObj.UpnpListInsert(&list, list_end_ptr, &list_inserted);
    ::std::cout << "DEBUG: insert element ----------------\n";
    ::std::cout << "DEBUG: list ref            = " << &list << ::std::endl;
    ::std::cout << "DEBUG: list begin          = "
                << listObj.UpnpListBegin(&list) << ::std::endl;
    ::std::cout << "DEBUG: list end            = " << listObj.UpnpListEnd(&list)
                << ::std::endl;
    ::std::cout << "DEBUG: list next           = " << list.next << ::std::endl;
    ::std::cout << "DEBUG: list prev           = " << list.prev << ::std::endl;
    ::std::cout << "DEBUG: list_inserted ref   = " << &list_inserted
                << ::std::endl;
    ::std::cout << "DEBUG: list_inserted begin = "
                << listObj.UpnpListBegin(&list_inserted) << ::std::endl;
    ::std::cout << "DEBUG: list_inserted  end  = "
                << listObj.UpnpListEnd(&list_inserted) << ::std::endl;
    ::std::cout << "DEBUG: list_inserted next  = " << list_inserted.next
                << ::std::endl;
    ::std::cout << "DEBUG: list inserted prev  = " << list_inserted.prev
                << ::std::endl;

    // Erasing just inserted element should give the initialized list again
    UpnpListIter list_erased_ptr =
        listObj.UpnpListErase(&list, list_inserted_ptr);
    ::std::cout << "DEBUG: erase element -----------------\n";
    ::std::cout << "DEBUG: list ref       = " << &list << ::std::endl;
    ::std::cout << "DEBUG: list begin     = " << listObj.UpnpListBegin(&list)
                << ::std::endl;
    ::std::cout << "DEBUG: list end       = " << listObj.UpnpListEnd(&list)
                << ::std::endl;
    ::std::cout << "DEBUG: list next      = " << list.next << ::std::endl;
    ::std::cout << "DEBUG: list prev      = " << list.prev << ::std::endl;
}
#endif

TEST(ListTestSuite, UpnpList_init_insert_erase)
// These are the tests for the results seen with test "UpnpList_show_entries"
// above.
{
    UpnpListHead list{};

    // Initialize list
    Clist listObj{};
    listObj.UpnpListInit(&list);
    EXPECT_EQ(listObj.UpnpListBegin(&list), &list);
    EXPECT_EQ(listObj.UpnpListEnd(&list), &list);
    EXPECT_EQ(list.next, &list);
    EXPECT_EQ(list.prev, &list);

    // Insert element before end of list
    UpnpListHead list_inserted{};
    UpnpListIter list_end_ptr = listObj.UpnpListEnd(&list);
    UpnpListIter list_inserted_ptr =
        listObj.UpnpListInsert(&list, list_end_ptr, &list_inserted);
    // Entries in list
    EXPECT_EQ(listObj.UpnpListBegin(&list), list_inserted_ptr);
    EXPECT_EQ(listObj.UpnpListEnd(&list), list_end_ptr);
    EXPECT_EQ(list.next, list_inserted_ptr);
    EXPECT_EQ(list.prev, list_inserted_ptr);
    // Entries in list_inserted
    EXPECT_EQ(listObj.UpnpListBegin(&list_inserted), list_end_ptr);
    EXPECT_EQ(listObj.UpnpListEnd(&list_inserted), list_inserted_ptr);
    EXPECT_EQ(list_inserted.next, list_end_ptr);
    EXPECT_EQ(list_inserted.prev, list_end_ptr);

    // Erasing just inserted element should give the initialized list again
    UpnpListIter list_erased_ptr =
        listObj.UpnpListErase(&list, list_inserted_ptr);
    EXPECT_EQ(list_erased_ptr, &list);
    // Same as UpnpListInit
    EXPECT_EQ(listObj.UpnpListBegin(&list), &list);
    EXPECT_EQ(listObj.UpnpListEnd(&list), &list);
    EXPECT_EQ(list.next, &list);
    EXPECT_EQ(list.prev, &list);
}

TEST(ListDeathTest, UpnpListInit_with_nullptr_to_list) {
    Clist listObj{};

    if (old_code) {
        std::cout << CRED "[ BUG      ]" CRES
                  << " A nullptr to a list must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(listObj.UpnpListInit(nullptr), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((listObj.UpnpListInit(nullptr), exit(0)), ExitedWithCode(0),
                    ".*");
    }
}

TEST(ListDeathTest, UpnpListBegin_with_nullptr_to_list) {
    UpnpListHead list{};

    Clist listObj{};
    listObj.UpnpListInit(&list);

    if (old_code) {
        std::cout << CRED "[ BUG      ]" CRES
                  << " A nullptr to a list must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(listObj.UpnpListBegin(nullptr), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((listObj.UpnpListBegin(nullptr), exit(0)),
                    ExitedWithCode(0), ".*");
        UpnpListIter ret_UpnpListBegin;
        memset(&ret_UpnpListBegin, 0xAA, sizeof(ret_UpnpListBegin));
        ret_UpnpListBegin = listObj.UpnpListBegin(nullptr);
        EXPECT_EQ(ret_UpnpListBegin, nullptr);
    }
}

TEST(ListTestSuite, UpnpListEnd) {
    UpnpListHead list{};

    Clist listObj{};
    listObj.UpnpListInit(&list);

    EXPECT_EQ(listObj.UpnpListEnd(&list), list.prev);
}

TEST(ListDeathTest, UpnpListEnd_with_nullptr_to_list) {
    UpnpListHead list{};

    Clist listObj{};
    listObj.UpnpListInit(&list);

    UpnpListIter ret{};
    ASSERT_EXIT((ret = listObj.UpnpListEnd(nullptr), exit(0)),
                ::testing::ExitedWithCode(0), ".*")
        << "  # A nullptr to a list must not segfault.";
    EXPECT_EQ(ret, nullptr);
}

TEST(ListTestSuite, UpnpListNext) {
    Clist listObj{};

    UpnpListHead list{};
    listObj.UpnpListInit(&list);
    UpnpListIter list_end_ptr = listObj.UpnpListEnd(&list);

    // Insert element before end of list
    UpnpListHead list_inserted{};
    UpnpListIter list_inserted_ptr =
        listObj.UpnpListInsert(&list, list_end_ptr, &list_inserted);

    // We have inserted between begin and end. So next from begin must be
    // inserted.
    UpnpListIter list_begin_ptr = listObj.UpnpListEnd(&list);
    UpnpListIter list_next_ptr = listObj.UpnpListNext(&list, list_begin_ptr);
    EXPECT_EQ(list_next_ptr, list_inserted_ptr);
}

TEST(ListTestSuite, UpnpListNext_with_nullptr_to_list) {
    UpnpListHead list{};

    Clist listObj{};
    listObj.UpnpListInit(&list);
    UpnpListIter list_end_ptr = listObj.UpnpListEnd(&list);

    // The first argument is ignored. We should find the list end.
    UpnpListIter list_next_ptr = listObj.UpnpListNext(nullptr, &list);
    EXPECT_EQ(list_next_ptr, list_end_ptr);
}

TEST(ListDeathTest, UpnpListNext_with_nullptr_to_position) {
    UpnpListHead list{};

    Clist listObj{};
    listObj.UpnpListInit(&list);

    if (old_code) {
        std::cout << CRED "[ BUG      ]" CRES
                  << " A nullptr to a position must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(listObj.UpnpListNext(&list, nullptr), ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT((listObj.UpnpListNext(&list, nullptr), exit(0)),
                    ExitedWithCode(0), ".*");
        UpnpListIter ret_UpnpListNext;
        memset(&ret_UpnpListNext, 0xAA, sizeof(ret_UpnpListNext));
        ret_UpnpListNext = listObj.UpnpListNext(&list, nullptr);
        EXPECT_EQ(ret_UpnpListNext, nullptr);
    }
}

TEST(ListTestSuite, UpnpListNext_with_begin_pos_on_initialized_list) {
    UpnpListHead list{};

    Clist listObj{};
    listObj.UpnpListInit(&list);

    // On an initialized list we have begin == end. The next position should
    // give the end position.
    UpnpListIter list_begin_ptr = listObj.UpnpListBegin(&list);
    UpnpListIter list_end_ptr = listObj.UpnpListEnd(&list);
    EXPECT_EQ(list_begin_ptr, list_end_ptr);
    UpnpListIter list_next_ptr = listObj.UpnpListNext(&list, list_begin_ptr);
    EXPECT_EQ(list_next_ptr, list_end_ptr);
}

TEST(ListTestSuite, UpnpListNext_with_invalid_position) {
    UpnpListHead list{};
    UpnpListHead list_invalid{};

    Clist listObj{};
    listObj.UpnpListInit(&list);

    UpnpListIter list_next_ptr = listObj.UpnpListNext(&list, &list_invalid);
    EXPECT_EQ(list_next_ptr, nullptr);
}

TEST(ListTestSuite, UpnpListInsert_with_nullptr_to_list) {
    UpnpListHead list{};
    UpnpListHead list_inserted{};

    Clist listObj{};
    listObj.UpnpListInit(&list);
    UpnpListIter list_end_ptr = listObj.UpnpListEnd(&list);

    // The first argument is ignored. We should find inserted next to begin (see
    // other test).
    UpnpListIter list_inserted_ptr =
        listObj.UpnpListInsert(nullptr, list_end_ptr, &list_inserted);
    EXPECT_NE(list_inserted_ptr, nullptr);
}

TEST(ListDeathTest, UpnpListInsert_with_nullptr_to_position) {
    UpnpListHead list{};
    UpnpListHead list_inserted{};

    Clist listObj{};
    listObj.UpnpListInit(&list);

    if (old_code) {
        std::cout << CRED "[ BUG      ]" CRES
                  << " A nullptr to a position must not segfault.\n";
        // This expects segfault.
        EXPECT_DEATH(listObj.UpnpListInsert(nullptr, nullptr, &list_inserted),
                     ".*");

    } else {

        // This expects NO segfault.
        ASSERT_EXIT(
            (listObj.UpnpListInsert(nullptr, nullptr, &list_inserted), exit(0)),
            ExitedWithCode(0), ".*");
        UpnpListIter ret_UpnpListInsert;
        memset(&ret_UpnpListInsert, 0xAA, sizeof(ret_UpnpListInsert));
        ret_UpnpListInsert =
            listObj.UpnpListInsert(nullptr, nullptr, &list_inserted);
        EXPECT_EQ(ret_UpnpListInsert, nullptr);
    }
}

TEST(ListTestSuite, UpnpListInsert_with_pos_on_list_begin) {
    UpnpListHead list{};
    UpnpListHead list_inserted{};

    Clist listObj{};
    listObj.UpnpListInit(&list);
    UpnpListIter list_begin_ptr = listObj.UpnpListBegin(&list);

    // The first argument is ignored. The inserted element should become the
    // begin.
    UpnpListIter list_inserted_ptr =
        listObj.UpnpListInsert(nullptr, list_begin_ptr, &list_inserted);
    list_begin_ptr = listObj.UpnpListBegin(&list);
    EXPECT_EQ(list_inserted_ptr, list_begin_ptr);
}

TEST(ListTestSuite, UpnpListErase_with_position_to_begin) {
    UpnpListHead list{};

    Clist listObj{};
    listObj.UpnpListInit(&list);
    UpnpListIter list_begin_ptr = listObj.UpnpListBegin(&list);
    UpnpListIter list_end_ptr = listObj.UpnpListEnd(&list);

    // The first argument is ignored.
    UpnpListIter list_next_ptr = listObj.UpnpListErase(nullptr, list_begin_ptr);
    EXPECT_EQ(list_next_ptr, list_end_ptr);
}

TEST(ListTestSuite, UpnpListErase_with_position_to_end) {
    UpnpListHead list{};

    Clist listObj{};
    listObj.UpnpListInit(&list);
    UpnpListIter list_end_ptr = listObj.UpnpListEnd(&list);

    // The first argument is ignored.
    UpnpListIter list_next_ptr = listObj.UpnpListErase(nullptr, list_end_ptr);
    EXPECT_EQ(list_next_ptr, list_end_ptr);
}

} // namespace utest

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
#include "gtest_main.inc"
    return gtest_return_code; // managed in gtest_main.inc
}
