#ifndef COMPA_LINKED_LIST_HPP
#define COMPA_LINKED_LIST_HPP
/*******************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 * Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
 * Redistribution only with this Copyright remark. Last modified: 2024-03-06
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * * Neither name of Intel Corporation nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/
/*!
 * \file
 * \ingroup threadutil
 * \brief Manage a linked list (for internal use only).
 *
 * Because this is for internal use, parameters are NOT checked for validity.
 * The caller must ensure valid parameters.
 */

#include "FreeList.hpp"

/// \brief Error condition for "out of memory".
#define EOUTOFMEM (-7 & 1 << 29)

/*! \brief Function for freeing list items. */
typedef void (*free_function)(void* arg);

/*! \brief Function for comparing list items. Returns 1 if itemA==itemB */
typedef int (*cmp_routine)(void* itemA, void* itemB);

/*! \brief Linked list node. Stores generic item and pointers to next and prev.
 * \internal
 */
struct ListNode {
    struct ListNode* prev;
    struct ListNode* next;
    void* item;
};

/*!
 * \brief Linked list (no protection).
 *
 * The first item of the list is stored at node: head->next\n
 * The last item of the list is stored at node: tail->prev\n
 * If head->next=tail, then list is empty.\n
 * To iterate through the list:
 *
 *     LinkedList g;
 *     ListNode *temp = NULL;
 *     for (temp = ListHead(g);temp!=NULL;temp = ListNext(g,temp)) {
 *     }
 *
 * \internal
 */
struct LinkedList {
    /*! \brief head, first item is stored at: head->next */
    ListNode head;
    /*! \brief tail, last item is stored at: tail->prev  */
    ListNode tail;
    /*! \brief size of list */
    long size;
    /*! \brief free list to use */
    FreeList freeNodeList;
    /*! \brief free function to use */
    free_function free_func;
    /*! \brief compare function to use */
    cmp_routine cmp_func;
};

/*!
 * \brief Initializes LinkedList. Must be called first and only once for List.
 *
 * \returns
 *  On success: **0**\n
 *  On error: EOUTOFMEM
 */
int ListInit(
    /*! [in] Must be valid, non null, pointer to a linked list. */
    LinkedList* list,
    /*! [in] Function used to compare items. (May be NULL). */
    cmp_routine cmp_func,
    /*! [in] Function used to free items. (May be NULL). */
    free_function free_func);

/*!
 * \brief Adds a node to the head of the list. Node gets immediately after
 * list head.
 *
 *  Precondition:
 *      The list has been initialized.
 *
 * \returns
 *  On success: The pointer to the ListNode.\n
 *  On error: nullptr
 */
ListNode* ListAddHead(
    /*! [in] Must be valid, non null, pointer to a linked list. */
    LinkedList* list,
    /*! [in] Item to be added. */
    void* item);

/*!
 * \brief Adds a node to the tail of the list. Node gets added immediately
 * before list.tail.
 *
 * Precondition: The list has been initialized.
 *
 * \returns
 *  On success: The pointer to the ListNode.\n
 *  On error: nullptr
 */
ListNode* ListAddTail(
    /*! [in] Must be valid, non null, pointer to a linked list. */
    LinkedList* list,
    /*! [in] Item to be added. */
    void* item);

/*!
 * \brief Adds a node after the specified node. Node gets added immediately
 * after bnode.
 *
 *  Precondition: The list has been initialized.
 *
 * \returns
 *  On success: The pointer to the ListNode.\n
 *  On error: nullptr
 */
ListNode* ListAddAfter(
    /*! [in] Must be valid, non null, pointer to a linked list. */
    LinkedList* list,
    /*! [in] Item to be added. */
    void* item,
    /*! [in] Node to add after. */
    ListNode* bnode);

/*!
 * \brief Adds a node before the specified node. Node gets added immediately
 * before anode.
 *
 * Precondition: The list has been initialized.
 *
 * \returns
 *  On success: The pointer to the ListNode.\n
 *  On error: nullptr
 */
ListNode* ListAddBefore(
    /*! [in] Must be valid, non null, pointer to a linked list. */
    LinkedList* list,
    /*! [in] Item to be added. */
    void* item,
    /*! [in] Node to add in front of. */
    ListNode* anode);

/*!
 * \brief Removes a node from the list. The memory for the node is freed.
 *
 * Precondition: The list has been initialized.
 *
 * \returns
 *  On success: The pointer to the item stored in the node or nullptr if the
 * item is freed.
 */
void* ListDelNode(
    /*! [in] Must be valid, non null, pointer to a linked list. */
    LinkedList* list,
    /*! [in] Node to delete. */
    ListNode* dnode,
    /*! [in] if !0 then item is freed using free function. If 0 (or free
       function is NULL) then item is not freed. */
    int freeItem);

/*!
 * \brief Removes all memory associated with list nodes. Does not free
 * LinkedList *list.
 *
 * Precondition: The list has been initialized.
 *
 * \returns
 *  On success: **0**
 *  On error: EINVAL
 */
int ListDestroy(
    /*! [in] Must be valid, non null, pointer to a linked list. */
    LinkedList* list,
    /*! [in] If !0 then item is freed using free function. If 0 (or free
       function is NULL) then item is not freed. */
    int freeItem);

/*!
 * \brief Returns the head of the list.
 *
 * Precondition: The list has been initialized.
 *
 * \returns
 *  On success: The head of the list. nullptr if list is empty.
 */
ListNode* ListHead(
    /*! [0] Must be valid, non null, pointer to a linked list. */
    LinkedList* list);

/*!
 * \brief Returns the tail of the list.
 *
 * Precondition: The list has been initialized.
 *
 * \returns
 *  On success: The tail of the list. nullptr if list is empty.
 */
ListNode* ListTail(
    /*! [in] Must be valid, non null, pointer to a linked list. */
    LinkedList* list);

/*!
 * \brief Returns the next item in the list.
 *
 * Precondition: The list has been initialized.
 *
 * \returns
 *  On success: The next item in the list, nullptr if there are no more items in
 * list.
 */
ListNode* ListNext(
    /*! [in] Must be valid, non null, pointer to a linked list. */
    LinkedList* list,
    /*! [in] Node from the list. */
    ListNode* node);

/*!
 * \brief Returns the previous item in the list.
 *
 * Precondition: The list has been initialized.
 *
 * \returns
 *  On success: The previous item in the list, nullptr if there are no more
 * items in list.
 */
ListNode* ListPrev(
    /*! [in] Must be valid, non null, pointer to a linked list. */
    LinkedList* list,
    /*! [in] Node from the list. */
    ListNode* node);

/*!
 * \brief Finds the specified item in the list.
 *
 * Uses the compare function specified in ListInit. If compare function is
 * nullptr then compares items as pointers.
 *
 * Precondition: The list has been initialized.
 *
 * \returns
 *  On success: The node containing the item, nullptr if no node contains the
 * item.
 */
ListNode* ListFind(
    /*! [in] Must be valid, non null, pointer to a linked list. */
    LinkedList* list,
    /*! [in] The node to start from, nullptr if to start from beginning. */
    ListNode* start,
    /*! [in] The item to search for. */
    void* item);

/*!
 * \brief Returns the size of the list.
 *
 * Precondition: The list has been initialized.
 *
 * \returns
 *  On success: The number of items in the list.
 */
long ListSize(
    /*! [in] Must be valid, non null, pointer to a linked list. */
    LinkedList* list);

#endif /* COMPA_LINKED_LIST_HPP */
