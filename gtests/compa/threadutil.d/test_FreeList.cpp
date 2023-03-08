// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-03-08

#include "FreeList.hpp"

#include "umock/stdlib.hpp"
#include "gmock/gmock.h"

using ::testing::_;
using ::testing::Return;

//
// A Freelist works together with a linked list. If we add or delete nodes on a
// linked list normaly memory allocation and freeing is used. This is expensive.
// Once allocated memory blocks if freed from the linked list will be added to
// the freelist for later use again.
//
// Additional tests and an interface for the FreeList module can be found until
// git commit 48acf7e29f7c4650a1159b0a1bfcce29462302a8 for later use again. It
// isn't used here anymore.

namespace compa {
bool old_code{false}; // Managed in compa/gtest_main.inc

//
// Mocked system calls
// -------------------
class StdlibMock : public umock::StdlibInterface {
  public:
    virtual ~StdlibMock() override {}
    MOCK_METHOD(void*, malloc, (size_t size), (override));
    MOCK_METHOD(void*, calloc, (size_t nmemb, size_t size), (override));
    MOCK_METHOD(void*, realloc, (void* ptr, size_t size), (override));
    MOCK_METHOD(void, free, (void* ptr), (override));
};

//
// TestSuites
// ----------
TEST(FreeListTestSuite, init_and_destroy) {
    FreeList free_list{};

    // Test Unit
    EXPECT_EQ(FreeListInit(&free_list, sizeof(int), 3), 0);

    EXPECT_EQ(free_list.head, nullptr);
    EXPECT_EQ(free_list.element_size, sizeof(int));
    EXPECT_EQ(free_list.maxFreeListLength, 3);
    EXPECT_EQ(free_list.freeListLength, 0);

    EXPECT_EQ(FreeListDestroy(&free_list), 0);

    EXPECT_EQ(free_list.head, nullptr);
    EXPECT_EQ(free_list.element_size, sizeof(int));
    EXPECT_EQ(free_list.maxFreeListLength, 3);
    EXPECT_EQ(free_list.freeListLength, 0);
}

TEST(FreeListTestSuite, allocate_node_from_memory) {
    // Get chunk of memory. Because the freelist is empty it should be new
    // allocated. Provide resources
    FreeList free_list{};
    EXPECT_EQ(FreeListInit(&free_list, sizeof(int), 2), 0);
    FreeListNode newnode{};

    // Mocking
    StdlibMock mocked_stdlib;
    umock::Stdlib stdlib_injectObj(&mocked_stdlib);
    EXPECT_CALL(mocked_stdlib, malloc(free_list.element_size))
        .WillOnce(Return(&newnode));
    EXPECT_CALL(mocked_stdlib, free(&free_list)).Times(0);

    // Test Unit
    FreeListNode* freenode = (FreeListNode*)FreeListAlloc(&free_list);

    // Check
    ASSERT_EQ(freenode, &newnode);
    EXPECT_EQ(free_list.head, nullptr);
    EXPECT_EQ(free_list.element_size, sizeof(int));
    EXPECT_EQ(free_list.maxFreeListLength, 2);
    // A node is allocated from the memory, because the freelist does not
    // contain a free node (freeListLength = 0).
    EXPECT_EQ(free_list.freeListLength, 0);

    EXPECT_EQ(FreeListDestroy(&free_list), 0);

    EXPECT_EQ(free_list.head, nullptr);
    EXPECT_EQ(free_list.element_size, sizeof(int));
    EXPECT_EQ(free_list.maxFreeListLength, 2);
    EXPECT_EQ(free_list.freeListLength, 0);
}

TEST(FreeListTestSuite, allocate_node_from_freelist) {
    // Get chunk of memory. Because the freelist is not empty it should come
    // from the list. Provide resources
    FreeList free_list{};
    EXPECT_EQ(FreeListInit(&free_list, sizeof(int), 2), 0);
    FreeListNode newnode{};

    // Mocking
    StdlibMock mocked_stdlib;
    umock::Stdlib stdlib_injectObj(&mocked_stdlib);
    // Expect to free the new node on destroying the freelist
    EXPECT_CALL(mocked_stdlib, free(&newnode)).Times(0);

    // Get a node from the freelist. Because the freelist ist empty it is
    // allocated from the memory.
    EXPECT_CALL(mocked_stdlib, malloc(free_list.element_size))
        .WillOnce(Return(&newnode));
    FreeListNode* freenode1 = (FreeListNode*)FreeListAlloc(&free_list);
    ASSERT_EQ(freenode1, &newnode);

    // Free the new node
    EXPECT_EQ(FreeListFree(&free_list, freenode1), 0);

    // Test Unit
    EXPECT_CALL(mocked_stdlib, malloc(free_list.element_size)).Times(0);
    FreeListNode* freenode2 = (FreeListNode*)FreeListAlloc(&free_list);
    ASSERT_EQ(freenode2, &newnode);

    // The freelist should be empty now.
    EXPECT_EQ(free_list.head, nullptr);
    EXPECT_EQ(free_list.element_size, sizeof(int));
    EXPECT_EQ(free_list.maxFreeListLength, 2);
    EXPECT_EQ(free_list.freeListLength, 0);

    // This should not call ::free() because there is no node in the freelist.
    EXPECT_EQ(FreeListDestroy(&free_list), 0);
}

TEST(FreeListTestSuite, add_unused_node_to_freelist) {
    FreeList free_list{};
    EXPECT_EQ(FreeListInit(&free_list, sizeof(int), 2), 0);
    FreeListNode newnode{};

    // Mocking
    StdlibMock mocked_stdlib;
    umock::Stdlib stdlib_injectObj(&mocked_stdlib);
    // Expect to get a new node from the memory
    EXPECT_CALL(mocked_stdlib, malloc(free_list.element_size))
        .WillOnce(Return(&newnode));
    // Expect to free the new node on destroying the freelist
    EXPECT_CALL(mocked_stdlib, free(&newnode)).Times(1);

    // Get a node from the freelist. Because the freelist ist empty it is
    // allocated from the memory.
    FreeListNode* freenode = (FreeListNode*)FreeListAlloc(&free_list);
    ASSERT_EQ(freenode, &newnode);

    // Test Unit
    EXPECT_EQ(FreeListFree(&free_list, freenode), 0);

    // The freed node should be added to the freelist for later use again.
    EXPECT_EQ(free_list.head, freenode);
    EXPECT_EQ(free_list.element_size, sizeof(int));
    EXPECT_EQ(free_list.maxFreeListLength, 2);
    EXPECT_EQ(free_list.freeListLength, 1);

    // This should call ::free() to free the node in the freelist.
    EXPECT_EQ(FreeListDestroy(&free_list), 0);
}

TEST(FreeListTestSuite, return_unused_node_to_operating_system) {
    FreeList free_list{};
    // We initialize the freelist with max size = 0 so a freed node will not be
    // added to the list.
    EXPECT_EQ(FreeListInit(&free_list, sizeof(int), 0), 0);
    FreeListNode newnode{};

    // Mocking
    StdlibMock mocked_stdlib;
    umock::Stdlib stdlib_injectObj(&mocked_stdlib);
    // Expect to get a new node from the memory
    EXPECT_CALL(mocked_stdlib, malloc(free_list.element_size))
        .WillOnce(Return(&newnode));
    // Expect to free the new node when giving it back to the operating system.
    EXPECT_CALL(mocked_stdlib, free(&newnode)).Times(1);

    // Get a node from the freelist. Because the freelist ist empty it is
    // allocated from the memory.
    FreeListNode* freenode = (FreeListNode*)FreeListAlloc(&free_list);
    ASSERT_EQ(freenode, &newnode);

    // Test Unit
    // This should the allocated node given back to the operating system, means
    // ::free() is called. It is not added to the freelist.
    EXPECT_EQ(FreeListFree(&free_list, freenode), 0);

    // The freelist is still empty.
    EXPECT_EQ(free_list.head, nullptr);
    EXPECT_EQ(free_list.element_size, sizeof(int));
    EXPECT_EQ(free_list.maxFreeListLength, 0);
    EXPECT_EQ(free_list.freeListLength, 0);

    EXPECT_EQ(FreeListDestroy(&free_list), 0);
}

#ifdef NDEBUG
TEST(FreeListTestSuite, freelist_init_is_nullptr) {
    EXPECT_EQ(FreeListInit(nullptr, sizeof(int), 3), EINVAL);
}
#else
TEST(FreeListDeathTest, freelist_init_is_nullptr) {
    // This expects failed assertion.
    EXPECT_DEATH(FreeListInit(nullptr, sizeof(int), 3), ".*");
}
#endif

TEST(FreeListTestSuite, freelist_init_with_zero_element_size) {
    FreeList free_list{};
    EXPECT_EQ(FreeListInit(&free_list, 0, 3), 0);
    EXPECT_EQ(free_list.element_size, (size_t)0);

    FreeListNode* freenode1 = (FreeListNode*)FreeListAlloc(&free_list);
    EXPECT_NE(freenode1, nullptr);
    EXPECT_EQ(free_list.head, nullptr);

    EXPECT_EQ(FreeListFree(&free_list, freenode1), 0);
    EXPECT_EQ(free_list.head, freenode1);

    FreeListNode* freenode2 = (FreeListNode*)FreeListAlloc(&free_list);
    EXPECT_EQ(freenode1, freenode2);

    EXPECT_EQ(FreeListDestroy(&free_list), 0);
}

#ifdef NDEBUG
TEST(FreeListTestSuite, freelist_alloc_from_a_nullptr) {
    EXPECT_EQ(FreeListAlloc(nullptr), nullptr);
}
#else
TEST(FreeListDeathTest, freelist_alloc_from_a_nullptr) {
    // This expects failed assertion.
    EXPECT_DEATH(FreeListAlloc(nullptr), ".*");
}
#endif

TEST(FreeListTestSuite, freelist_with_nullptr_to_element) {
    FreeList free_list{};
    EXPECT_EQ(FreeListInit(&free_list, 4, 3), 0);

    // Mocking
    StdlibMock mocked_stdlib;
    umock::Stdlib stdlib_injectObj(&mocked_stdlib);
    EXPECT_CALL(mocked_stdlib, free(nullptr)).Times(1);

    // Test Unit
    EXPECT_EQ(FreeListFree(&free_list, nullptr), 0);
}

#ifdef NDEBUG
TEST(FreeListTestSuite, freelist_destroy_a_nullptr) {
    EXPECT_EQ(FreeListDestroy(nullptr), EINVAL);
}
#else
TEST(FreeListDeathTest, freelist_destroy_a_nullptr) {
    // This expects failed assertion.
    EXPECT_DEATH(FreeListDestroy(nullptr), ".*");
}
#endif

} // namespace compa

//
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "compa/gtest_main.inc"
    return gtest_return_code; // managed in compa/gtest_main.inc
}
