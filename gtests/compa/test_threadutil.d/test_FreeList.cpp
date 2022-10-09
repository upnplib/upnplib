// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-10-12

#include "FreeList.hpp"

#include "upnplib/mocking/stdlib.hpp"
#include "gmock/gmock.h"

using ::testing::_;
using ::testing::Return;

namespace mock = upnplib::mocking;

namespace compa {
bool old_code{false}; // Managed in upnplib_gtest_main.inc

//
// Mocked system calls
// -------------------
class StdlibMock : public mock::StdlibInterface {
  public:
    virtual ~StdlibMock() override {}
    MOCK_METHOD(void*, malloc, (size_t size), (override));
    MOCK_METHOD(void, free, (void* ptr), (override));
    MOCK_METHOD(void*, calloc, (size_t nmemb, size_t size), (override));
};

//
TEST(FreeListTestSuite, init_and_destroy) {
    FreeList free_list{};

    // Test Unit
    EXPECT_EQ(FreeListInit(&free_list, sizeof(int), 3), 0);

    EXPECT_EQ(free_list.head, nullptr);
    EXPECT_EQ(free_list.element_size, (size_t)4);
    EXPECT_EQ(free_list.maxFreeListLength, 3);
    EXPECT_EQ(free_list.freeListLength, 0);

    EXPECT_EQ(FreeListDestroy(&free_list), 0);
}

TEST(FreeListTestSuite, allocate_memory) {
    // Get chunk of memory. Because the freelist is empty it should be new
    // allocated. Provide resources
    FreeList free_list{};
    EXPECT_EQ(FreeListInit(&free_list, sizeof(int), 3), 0);
    FreeListNode newnode{};

    // Mocking
    StdlibMock mocked_stdlib;
    mock::Stdlib stdlib_injectObj(&mocked_stdlib);
    EXPECT_CALL(mocked_stdlib, malloc(free_list.element_size))
        .WillOnce(Return(&newnode));

    // Test Unit
    FreeListNode* freenode = (FreeListNode*)FreeListAlloc(&free_list);

    // Check
    ASSERT_EQ(freenode, &newnode);
    EXPECT_EQ(free_list.head, nullptr);
    EXPECT_EQ(free_list.element_size, (size_t)4);
    EXPECT_EQ(free_list.maxFreeListLength, 3);
    EXPECT_EQ(free_list.freeListLength, 0);
}

} // namespace compa

//
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "compa/gtest_main.inc"
}
