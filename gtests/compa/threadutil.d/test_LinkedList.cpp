// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-01-30

#include "umock/stdlib.hpp"
#include "gmock/gmock.h"

#include "LinkedList.hpp"

using ::testing::_;
using ::testing::Return;

namespace compa {

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
//###############################
// Linked List Testsuite        #
//###############################

// Interface for the LinkedList module
// -----------------------------------
class ILinkedList {
  public:
    virtual ~ILinkedList() {}
    virtual int ListInit(LinkedList* list, cmp_routine cmp_func,
                         free_function free_func) = 0;
    virtual ListNode* ListAddHead(LinkedList* list, void* item) = 0;
    virtual ListNode* ListAddTail(LinkedList* list, void* item) = 0;
    virtual ListNode* ListAddAfter(LinkedList* list, void* item,
                                   ListNode* bnode) = 0;
    virtual ListNode* ListAddBefore(LinkedList* list, void* item,
                                    ListNode* anode) = 0;
    virtual void* ListDelNode(LinkedList* list, ListNode* dnode,
                              int freeItem) = 0;
    virtual int ListDestroy(LinkedList* list, int freeItem) = 0;
    virtual ListNode* ListHead(LinkedList* list) = 0;
    virtual ListNode* ListTail(LinkedList* list) = 0;
    virtual ListNode* ListNext(LinkedList* list, ListNode* node) = 0;
    virtual ListNode* ListPrev(LinkedList* list, ListNode* node) = 0;
    virtual ListNode* ListFind(LinkedList* list, ListNode* start,
                               void* item) = 0;
    virtual long ListSize(LinkedList* list) = 0;
};

class CLinkedList : public ILinkedList {
  public:
    virtual ~CLinkedList() {}

    virtual int ListInit(LinkedList* list, cmp_routine cmp_func,
                         free_function free_func) override {
        return ::ListInit(list, cmp_func, free_func);
    }
    virtual ListNode* ListAddHead(LinkedList* list, void* item) override {
        return ::ListAddHead(list, item);
    }
    virtual ListNode* ListAddTail(LinkedList* list, void* item) override {
        return ::ListAddTail(list, item);
    }
    virtual ListNode* ListAddAfter(LinkedList* list, void* item,
                                   ListNode* bnode) override {
        return ::ListAddAfter(list, item, bnode);
    }
    virtual ListNode* ListAddBefore(LinkedList* list, void* item,
                                    ListNode* anode) override {
        return ::ListAddBefore(list, item, anode);
    }
    virtual void* ListDelNode(LinkedList* list, ListNode* dnode,
                              int freeItem) override {
        return ::ListDelNode(list, dnode, freeItem);
    }
    virtual int ListDestroy(LinkedList* list, int freeItem) override {
        return ::ListDestroy(list, freeItem);
    }
    virtual ListNode* ListHead(LinkedList* list) override {
        return ::ListHead(list);
    }
    virtual ListNode* ListTail(LinkedList* list) override {
        return ::ListTail(list);
    }
    virtual ListNode* ListNext(LinkedList* list, ListNode* node) override {
        return ::ListNext(list, node);
    }
    virtual ListNode* ListPrev(LinkedList* list, ListNode* node) override {
        return ::ListPrev(list, node);
    }
    virtual ListNode* ListFind(LinkedList* list, ListNode* start,
                               void* item) override {
        return ::ListFind(list, start, item);
    }
    virtual long ListSize(LinkedList* list) override {
        return ::ListSize(list);
    }
};

//
// Testsuite for the LinkedList module
// -----------------------------------
class LinkedListTestSuite : public ::testing::Test {
  protected:
    // Member variables: instantiate the module object
    CLinkedList LinkedListObj{};
    LinkedList m_linked_list{};

    // instantiate the mock objects.
    StdlibMock mocked_stdlib{};
};

TEST_F(LinkedListTestSuite, ListInit_ListDestroy_empty_list) {
    // Expectations: nothing should happen with malloc() and free()
    umock::Stdlib stdlib_injectObj(&mocked_stdlib);
    EXPECT_CALL(mocked_stdlib, malloc(_)).Times(0);
    EXPECT_CALL(mocked_stdlib, free(_)).Times(0);

    // Initialize linked list
    ASSERT_EQ(LinkedListObj.ListInit(&m_linked_list, NULL, NULL), 0);
    EXPECT_EQ(LinkedListObj.ListSize(&m_linked_list), 0);

    // Destroy linked list
    EXPECT_EQ(LinkedListObj.ListDestroy(&m_linked_list, 0), 0);
}

TEST_F(LinkedListTestSuite, ListAddTail_with_item) {
    // The list node contains a pointer to the item
    ListNode node1{};
    std::string item1 = "item1";

    // Expectations
    umock::Stdlib stdlib_injectObj(&mocked_stdlib);
    EXPECT_CALL(mocked_stdlib, malloc(sizeof(ListNode)))
        // malloc() is expected to be called by ListAddTail.
        .WillOnce(Return(&node1));
    // free() is expected to be called by ListDestroy.
    EXPECT_CALL(mocked_stdlib, free(&node1)).Times(1);

    // Initialize linked list
    ASSERT_EQ(LinkedListObj.ListInit(&m_linked_list, NULL, NULL), 0);

    // Add node to the list tail, should call malloc() one time for node1.
    ListNode* list_node1 = LinkedListObj.ListAddTail(&m_linked_list, &item1);
    EXPECT_EQ(list_node1, &node1);
    EXPECT_EQ(LinkedListObj.ListSize(&m_linked_list), 1);
    // Look at the item in the ListNode
    EXPECT_EQ(*(std::string*)list_node1->item, "item1");

    // Destroy linked list, should call free() one time for node1.
    EXPECT_EQ(LinkedListObj.ListDestroy(&m_linked_list, 0), 0);
}

TEST_F(LinkedListTestSuite, ListDelNode_from_list_with_one_node) {
    // The list node contains a pointer to the item
    ListNode node1{};
    std::string item1 = "item1";

    // Expectations
    umock::Stdlib stdlib_injectObj(&mocked_stdlib);
    EXPECT_CALL(mocked_stdlib, malloc(sizeof(ListNode)))
        .WillOnce(Return(&node1));
    // We do not expect that free() is called because the deleted node is added
    // to the free list.
    EXPECT_CALL(mocked_stdlib, free(&node1)).Times(0);

    // Initialize linked list
    ASSERT_EQ(LinkedListObj.ListInit(&m_linked_list, NULL, NULL), 0);

    // Add node to the list head, should call malloc() one time for node1.
    ListNode* list_node1 = LinkedListObj.ListAddHead(&m_linked_list, &item1);
    EXPECT_EQ(list_node1, &node1);
    EXPECT_EQ(LinkedListObj.ListSize(&m_linked_list), 1);
    EXPECT_EQ(*(std::string*)list_node1->item, "item1");

    // Delete node from list, check returned pointer to item1. This does not
    // call free() because the deleted node is added to the free list instead.
    std::string* item1ptr =
        (std::string*)LinkedListObj.ListDelNode(&m_linked_list, list_node1, 0);
    EXPECT_EQ(item1ptr, &item1);
    EXPECT_EQ(*item1ptr, "item1");
    EXPECT_EQ(LinkedListObj.ListSize(&m_linked_list), 0);

    // We do not destroy the list to check that free() isn't called by
    // ListDelNode. This may cause a memory leak of one sizeof(ListNode) == 24
    // bytes. This is acaptable for this test condition.
    // EXPECT_EQ(LinkedListObj.ListDestroy(&m_linked_list, 0), 0);
}

TEST_F(LinkedListTestSuite, use_all_functions_on_one_initialized_list) {
    ListNode node1{};
    ListNode node2{};
    ListNode node3{};
    ListNode node4{};
    ListNode node5{};
    std::string item1 = "item1";
    std::string item2 = "item2";
    std::string item3 = "item3";
    std::string item4 = "item4";
    std::string item5 = "item5";

    // Expectations
    umock::Stdlib stdlib_injectObj(&mocked_stdlib);
    EXPECT_CALL(mocked_stdlib, malloc(sizeof(ListNode)))
        .WillOnce(Return(&node1))
        .WillOnce(Return(&node2))
        .WillOnce(Return(&node3))
        .WillOnce(Return(&node4))
        .WillOnce(Return(&node5));
    EXPECT_CALL(mocked_stdlib, free(&node1)).Times(1);
    EXPECT_CALL(mocked_stdlib, free(&node2)).Times(1);
    EXPECT_CALL(mocked_stdlib, free(&node3)).Times(1);
    EXPECT_CALL(mocked_stdlib, free(&node4)).Times(1);
    EXPECT_CALL(mocked_stdlib, free(&node5)).Times(1);

    // Initialize linked list
    ASSERT_EQ(LinkedListObj.ListInit(&m_linked_list, NULL, NULL), 0);

    // Add node to the list head, should call malloc() first time for node1.
    ListNode* list_node1 = LinkedListObj.ListAddHead(&m_linked_list, &item1);
    EXPECT_EQ(LinkedListObj.ListSize(&m_linked_list), 1);
    EXPECT_EQ(*(std::string*)list_node1->item, "item1");

    // Add node to the list tail, should call malloc() second time for node2.
    ListNode* list_node2 = LinkedListObj.ListAddTail(&m_linked_list, &item2);
    EXPECT_EQ(LinkedListObj.ListSize(&m_linked_list), 2);
    EXPECT_EQ(*(std::string*)list_node2->item, "item2");

    // Add node to the list tail, should call malloc() third time for node3.
    ListNode* list_node3 = LinkedListObj.ListAddTail(&m_linked_list, &item3);
    EXPECT_EQ(LinkedListObj.ListSize(&m_linked_list), 3);
    EXPECT_EQ(*(std::string*)list_node3->item, "item3");

    // Add node after node2, should call malloc() fourth time for node4.
    ListNode* list_node4 =
        LinkedListObj.ListAddAfter(&m_linked_list, &item4, list_node2);
    EXPECT_EQ(LinkedListObj.ListSize(&m_linked_list), 4);
    EXPECT_EQ(*(std::string*)list_node4->item, "item4");

    // Add node before node4, should call malloc() fifth time for node5.
    ListNode* list_node5 =
        LinkedListObj.ListAddBefore(&m_linked_list, &item5, list_node4);
    EXPECT_EQ(LinkedListObj.ListSize(&m_linked_list), 5);
    EXPECT_EQ(*(std::string*)list_node5->item, "item5");

    // Do you know how are the nodes ordered now in the list?
    // DEBUG: iterate through list and list its items
    // ListNode* temp = NULL;
    // for (temp = ListHead(&m_linked_list); temp != NULL;
    //      temp = ListNext(&m_linked_list, temp)) {
    //     std::cout << "DEBUG: listnode = " << temp << ", item("
    //               << (std::string*)temp->item
    //               << ") -> " << *(std::string*)temp->item << std::endl;
    // }

    // Check order of the items in the list
    ListNode* list_node = LinkedListObj.ListHead(&m_linked_list);
    EXPECT_EQ(*(std::string*)list_node->item, "item1");
    list_node = LinkedListObj.ListNext(&m_linked_list, list_node);
    EXPECT_EQ(*(std::string*)list_node->item, "item2");
    list_node = LinkedListObj.ListNext(&m_linked_list, list_node);
    EXPECT_EQ(*(std::string*)list_node->item, "item5");
    list_node = LinkedListObj.ListNext(&m_linked_list, list_node);
    EXPECT_EQ(*(std::string*)list_node->item, "item4");
    list_node = LinkedListObj.ListNext(&m_linked_list, list_node);
    EXPECT_EQ(*(std::string*)list_node->item, "item3");

    // The last list_node should be the tail now.
    EXPECT_EQ(LinkedListObj.ListTail(&m_linked_list), list_node);
    // Previous of the tail should be list_node4.
    list_node = LinkedListObj.ListPrev(&m_linked_list, list_node);
    EXPECT_EQ(list_node, list_node4);
    // Search item address from the beginning of the list.
    EXPECT_EQ(LinkedListObj.ListFind(&m_linked_list, NULL, list_node->item),
              list_node4);
    // Search item address starting on list_node2.
    EXPECT_EQ(
        LinkedListObj.ListFind(&m_linked_list, list_node2, list_node->item),
        list_node4);

    // Destroy linked list, should call free() 5 times with node addresses.
    EXPECT_EQ(LinkedListObj.ListDestroy(&m_linked_list, 0), 0);
}

//
// We need a wrapper for the free function to call the method. I haven't found
// the address of the member function to have the same signature than
// free_function so we could call it direct. See also
// https://stackoverflow.com/a/8865807/5014688
void free_func(void* t_free_func) { umock::stdlib_h.free(t_free_func); }

TEST_F(LinkedListTestSuite, ListDelNode_with_free_function) {
    ListNode node1{};
    std::string item1 = "item1";

    // Expectations
    umock::Stdlib stdlib_injectObj(&mocked_stdlib);
    EXPECT_CALL(mocked_stdlib, malloc(sizeof(ListNode)))
        .WillOnce(Return(&node1));
    // This will free the item on node1 when the node is deleted.
    EXPECT_CALL(mocked_stdlib, free(&item1)).Times(1);
    // This will free node1 from the FreeList when the LinkedList is destroyed.
    EXPECT_CALL(mocked_stdlib, free(&node1)).Times(1);

    // Initialize linked list with free function.
    ASSERT_EQ(
        LinkedListObj.ListInit(&m_linked_list, NULL, (free_function)free_func),
        0);

    // Add node to the list tail, should call malloc().
    ListNode* list_node1 = LinkedListObj.ListAddTail(&m_linked_list, &item1);

    // Delete node from list. This does not call free() for the node because the
    // deleted node is added to the free list instead. But it calls free() for
    // its item which is the first expected call (see above).
    std::string* node1_item = (std::string*)LinkedListObj.ListDelNode(
        &m_linked_list, list_node1, true);
    EXPECT_EQ(node1_item, nullptr);
    EXPECT_EQ(LinkedListObj.ListSize(&m_linked_list), 0);

    // Destroy linked list, should call free() for the node.
    EXPECT_EQ(LinkedListObj.ListDestroy(&m_linked_list, 0), 0);
}

//
// Function for comparing list items. Returns 1 if itemA==itemB
// typedef int (*cmp_routine)(void *itemA, void *itemB);
int cmp_func(std::string* t_itemA, std::string* t_itemB) {
    return *t_itemA == *t_itemB;
}

TEST_F(LinkedListTestSuite, ListFind_with_comparing_items) {
    ListNode node1{};
    ListNode node2{};
    ListNode node3{};
    std::string item1 = "item1";
    std::string item2 = "item2";
    std::string item3 = "item3";

    // Check compare function
    EXPECT_EQ(cmp_func(&item1, &item2), 0);
    EXPECT_EQ(cmp_func(&item1, &item1), 1);

    // Expectations
    umock::Stdlib stdlib_injectObj(&mocked_stdlib);
    EXPECT_CALL(mocked_stdlib, malloc(sizeof(ListNode)))
        .WillOnce(Return(&node1))
        .WillOnce(Return(&node2))
        .WillOnce(Return(&node3));
    EXPECT_CALL(mocked_stdlib, free(&node1)).Times(1);
    EXPECT_CALL(mocked_stdlib, free(&node2)).Times(1);
    EXPECT_CALL(mocked_stdlib, free(&node3)).Times(1);

    // Initialize linked list with compare function.
    ASSERT_EQ(
        LinkedListObj.ListInit(&m_linked_list, (cmp_routine)cmp_func, NULL), 0);

    // Add node to the list head, should call malloc() first time for node1.
    ListNode* list_node1 = LinkedListObj.ListAddTail(&m_linked_list, &item1);
    EXPECT_EQ(LinkedListObj.ListSize(&m_linked_list), 1);
    EXPECT_EQ(*(std::string*)list_node1->item, "item1");
    // Check compare function
    EXPECT_EQ(cmp_func(&item1, (std::string*)list_node1->item), 1);

    ListNode* list_node2 = LinkedListObj.ListAddTail(&m_linked_list, &item2);
    ListNode* list_node3 = LinkedListObj.ListAddTail(&m_linked_list, &item3);
    EXPECT_EQ(LinkedListObj.ListSize(&m_linked_list), 3);

    // Search item from the beginning of the list.
    EXPECT_EQ(LinkedListObj.ListFind(&m_linked_list, NULL, &item2), list_node2);
    // Search item starting from a node.
    EXPECT_EQ(LinkedListObj.ListFind(&m_linked_list, list_node2, &item3),
              list_node3);

    // Destroy linked list, should call free() for the nodes.
    EXPECT_EQ(LinkedListObj.ListDestroy(&m_linked_list, 0), 0);
}

} // namespace compa

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
