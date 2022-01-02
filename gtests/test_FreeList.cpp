// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-11-01

#include "upnpmock/stdlib.hpp"

#include "port.hpp"
#include "gmock/gmock.h"

#include "FreeList.cpp"

using ::testing::_;
using ::testing::Return;

namespace upnp {

//
// Mocked system calls
// -------------------
// See the respective include files in upnp/include/upnpmock/

class Mock_stdlib : public Bstdlib {
    // Class to mock the free system functions.
    Bstdlib* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_stdlib() {
        m_oldptr = stdlib_h;
        stdlib_h = this;
    }
    virtual ~Mock_stdlib() { stdlib_h = m_oldptr; }

    MOCK_METHOD(void*, malloc, (size_t size), (override));
    MOCK_METHOD(void, free, (void* ptr), (override));
};

//
//###############################
// Free List Testsuite          #
//###############################
// A Freelist works together with a linked list. If we add or delete nodes on a
// linked list normaly memory allocation and freeing is used. This is expensive.
// Once allocated memory blocks if freed from the linked list will be added to
// the freelist for later use again.

// Interface for the FreeList module
// ---------------------------------
class IFreeList {
  public:
    virtual ~IFreeList() {}
    virtual int FreeListInit(FreeList* free_list, size_t elementSize,
                             int maxFreeListLength) = 0;
    virtual void* FreeListAlloc(FreeList* free_list) = 0;
    virtual int FreeListFree(FreeList* free_list, void* element) = 0;
    virtual int FreeListDestroy(FreeList* free_list) = 0;
};

class CFreeList : public IFreeList {
  public:
    virtual ~CFreeList() {}

    virtual int FreeListInit(FreeList* free_list, size_t elementSize,
                             int maxFreeListLength) override {
        return ::FreeListInit(free_list, elementSize, maxFreeListLength);
    }
    virtual void* FreeListAlloc(FreeList* free_list) override {
        return ::FreeListAlloc(free_list);
    }
    virtual int FreeListFree(FreeList* free_list, void* element) override {
        return ::FreeListFree(free_list, element);
    }
    virtual int FreeListDestroy(FreeList* free_list) override {
        return ::FreeListDestroy(free_list);
    }
};

//
// Testsuite for the FreeList module
// ---------------------------------
class FreeListTestSuite : public ::testing::Test {
  protected:
    // Member variables: instantiate the module object
    CFreeList FreeListObj{};
    FreeList m_free_list{};

    // instantiate the mock objects.
    Mock_stdlib mocked_stdlib{};
};

TEST_F(FreeListTestSuite, init_alocate_free_destroy) {
    // initialize the freelist
    EXPECT_EQ(FreeListObj.FreeListInit(&m_free_list, sizeof(int), 3), 0);
    EXPECT_EQ(m_free_list.head, nullptr);
    EXPECT_EQ(m_free_list.element_size, (size_t)4);
    EXPECT_EQ(m_free_list.maxFreeListLength, 3);
    EXPECT_EQ(m_free_list.freeListLength, 0);

    // Get a new node. Because the freelist is empty it should be allocated from
    // memory.
    FreeListNode anynode0{};
    EXPECT_CALL(mocked_stdlib, malloc(sizeof(int))).WillOnce(Return(&anynode0));

    FreeListNode* newnode =
        (FreeListNode*)FreeListObj.FreeListAlloc(&m_free_list);
    EXPECT_EQ(newnode, &anynode0);
    EXPECT_EQ(m_free_list.head, nullptr);
    EXPECT_EQ(m_free_list.element_size, (size_t)4);
    EXPECT_EQ(m_free_list.maxFreeListLength, 3);
    EXPECT_EQ(m_free_list.freeListLength, 0);

    // Put an unused node to the freelist. It should increase the freelist.
    FreeListNode anynode1{};
    EXPECT_EQ(FreeListObj.FreeListFree(&m_free_list, &anynode1), 0);
    EXPECT_EQ(m_free_list.head, &anynode1);
    EXPECT_EQ(m_free_list.freeListLength, 1);

    // Put an unused node to the freelist. It should increase the freelist.
    FreeListNode anynode2{};
    EXPECT_EQ(FreeListObj.FreeListFree(&m_free_list, &anynode2), 0);
    EXPECT_EQ(m_free_list.head, &anynode2);

    // Put an unused node to the freelist. It should increase the freelist.
    FreeListNode anynode3{};
#ifdef OLD_TEST
    // The freelist is initialized with maxFreeListLength = 3 so it should add a
    // node to the list and not freeing its resource. The comparison of
    // freeListLength with maxFreeListLength is wrong by 1 node.
    EXPECT_EQ(m_free_list.freeListLength, 2);
    EXPECT_EQ(m_free_list.maxFreeListLength, 3);
    // freeing the resource is wrong.
    EXPECT_CALL(mocked_stdlib, free(&anynode3)).Times(1);
    EXPECT_EQ(FreeListObj.FreeListFree(&m_free_list, &anynode3), 0);
    std::cout << "  BUG! The resource of the node should not be freed but "
                 "instead added to the freelist.\n";
    // We will find the previous anynode2 not anynode3 in the freelist.
    EXPECT_EQ(m_free_list.head, &anynode2);
#else
    EXPECT_EQ(FreeListObj.FreeListFree(&m_free_list, &anynode3), 0);
    EXPECT_EQ(m_free_list.head, &anynode3);
    EXPECT_EQ(m_free_list.freeListLength, 3)
        << "  # The resource of the node should not be freed but instead added "
           "to the freelist.";
#endif

    // Getting a free node from the freelist should decrease its length
    newnode = (FreeListNode*)FreeListObj.FreeListAlloc(&m_free_list);
    EXPECT_EQ(newnode, &anynode2);
    EXPECT_EQ(m_free_list.head, &anynode1);
    EXPECT_EQ(m_free_list.freeListLength, 1);

    // Getting a free node from the freelist should decrease its length to zero
    // nodes now
    newnode = (FreeListNode*)FreeListObj.FreeListAlloc(&m_free_list);
    EXPECT_EQ(newnode, &anynode1);
    EXPECT_EQ(m_free_list.head, nullptr);
    EXPECT_EQ(m_free_list.freeListLength, 0);

    // Getting next free node from the freelist should be allocated from memory.
    EXPECT_CALL(mocked_stdlib, malloc(sizeof(int))).WillOnce(Return(&anynode1));

    newnode = (FreeListNode*)FreeListObj.FreeListAlloc(&m_free_list);
    EXPECT_EQ(newnode, &anynode1);
    EXPECT_EQ(m_free_list.head, nullptr);
    EXPECT_EQ(m_free_list.element_size, (size_t)4);
    EXPECT_EQ(m_free_list.maxFreeListLength, 3);
    EXPECT_EQ(m_free_list.freeListLength, 0);

    // Add some free nodes so we can destroy the freelist, newnode is &anynode1
    EXPECT_EQ(FreeListObj.FreeListFree(&m_free_list, newnode), 0);
    EXPECT_EQ(FreeListObj.FreeListFree(&m_free_list, &anynode2), 0);

    // Destroy the freelist with two nodes
    EXPECT_CALL(mocked_stdlib, free(_)).Times(2);

    EXPECT_EQ(FreeListObj.FreeListDestroy(&m_free_list), 0);
    EXPECT_EQ(m_free_list.head, nullptr);
    EXPECT_EQ(m_free_list.element_size, (size_t)4);
    EXPECT_EQ(m_free_list.maxFreeListLength, 3);
    EXPECT_EQ(m_free_list.freeListLength, 0);
}

TEST_F(FreeListTestSuite, freelist_for_0_size_nodes) {
    EXPECT_EQ(FreeListObj.FreeListInit(&m_free_list, 0, 3), 0);

    // Get node from freelist
    EXPECT_CALL(mocked_stdlib, malloc(0)).WillOnce(Return(nullptr));
    EXPECT_EQ(FreeListObj.FreeListAlloc(&m_free_list), nullptr);
    EXPECT_EQ(m_free_list.head, nullptr);
    EXPECT_EQ(m_free_list.element_size, (size_t)0);
    EXPECT_EQ(m_free_list.maxFreeListLength, 3);
    EXPECT_EQ(m_free_list.freeListLength, 0);

    // Add free node to freelist
    FreeListNode anynode1{};
    EXPECT_EQ(FreeListObj.FreeListFree(&m_free_list, &anynode1), 0);
    EXPECT_EQ(m_free_list.maxFreeListLength, 3);
    EXPECT_EQ(m_free_list.element_size, (size_t)0);
#ifdef OLD_TEST
    // It isn't possible to allocate a 0-sized memory block. malloc() returns a
    // nullptr. So it should also be impossible to add a 0-sized node to the
    // freelist to be available for later use.
    std::cout << "  BUG! Adding a 0-sized node to the freelist should not be "
                 "possible.\n";
    EXPECT_NE(m_free_list.head, nullptr);
    EXPECT_EQ(m_free_list.freeListLength, 1);
#else
    EXPECT_EQ(m_free_list.head, nullptr);
    EXPECT_EQ(m_free_list.freeListLength, 0)
        << "  # Adding a 0-sized node to the freelist should not be possible.";
#endif
}

TEST_F(FreeListTestSuite, destroy_freelist_for_0_size_nodes) {
    EXPECT_EQ(FreeListObj.FreeListInit(&m_free_list, 0, 3), 0);

    EXPECT_EQ(FreeListObj.FreeListDestroy(&m_free_list), 0);
    EXPECT_EQ(m_free_list.head, nullptr);
    EXPECT_EQ(m_free_list.element_size, (size_t)0);
    EXPECT_EQ(m_free_list.maxFreeListLength, 3);
    EXPECT_EQ(m_free_list.freeListLength, 0);
}

TEST_F(FreeListTestSuite, allocate_node_from_freelist_with_maximal_0_items) {
    EXPECT_EQ(FreeListObj.FreeListInit(&m_free_list, 4, 0), 0);

    EXPECT_EQ(m_free_list.head, nullptr);
    EXPECT_EQ(m_free_list.element_size, (size_t)4);
    EXPECT_EQ(m_free_list.maxFreeListLength, 0);
    EXPECT_EQ(m_free_list.freeListLength, 0);

    // Get node from freelist. This should be possible but never from the
    // freelist, only allocated from memory.
    FreeListNode anynode1{};
    EXPECT_CALL(mocked_stdlib, malloc(4)).WillOnce(Return(&anynode1));
    EXPECT_EQ(FreeListObj.FreeListAlloc(&m_free_list), &anynode1);
    EXPECT_EQ(m_free_list.head, nullptr);
    EXPECT_EQ(m_free_list.element_size, (size_t)4);
    EXPECT_EQ(m_free_list.maxFreeListLength, 0);
    EXPECT_EQ(m_free_list.freeListLength, 0);
}

TEST_F(FreeListTestSuite, put_free_node_to_freelist_with_maximal_0_items) {
    EXPECT_EQ(FreeListObj.FreeListInit(&m_free_list, 4, 0), 0);

    // Put free node to freelist. This should be possible but never to the
    // freelist, only freeing the memory block.
    FreeListNode anynode1{};
    EXPECT_CALL(mocked_stdlib, free(&anynode1)).Times(1);
    EXPECT_EQ(FreeListObj.FreeListFree(&m_free_list, &anynode1), 0);
    EXPECT_EQ(m_free_list.head, nullptr);
    EXPECT_EQ(m_free_list.element_size, (size_t)4);
    EXPECT_EQ(m_free_list.maxFreeListLength, 0);
    EXPECT_EQ(m_free_list.freeListLength, 0);
}

TEST_F(FreeListTestSuite, initialize_not_existing_freelist) {
    EXPECT_EQ(FreeListObj.FreeListInit(nullptr, 0, 0), EINVAL);
}

} // namespace upnp

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
