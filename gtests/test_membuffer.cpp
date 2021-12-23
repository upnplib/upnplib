// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-12-28

#include "gtest/gtest.h"
//#include "gmock/gmock.h"

#include "genlib/util/membuffer.cpp"

namespace upnp {

// Interface for the uri module
// ============================
// clang-format off

class Imembuffer {
  public:
    virtual ~Imembuffer() {}

    virtual char* str_alloc(
            const char* str, size_t str_len) = 0;
    virtual int memptr_cmp(
            memptr* m, const char* s) = 0;
    virtual int memptr_cmp_nocase(
            memptr* m, const char* s) = 0;
    virtual int membuffer_set_size(
            membuffer* m, size_t new_length) = 0;
    virtual void membuffer_init(
            membuffer* m) = 0;
    virtual void membuffer_destroy(
            membuffer* m) = 0;
    virtual int membuffer_assign(
            membuffer* m, const void* buf, size_t buf_len) = 0;
    virtual int membuffer_assign_str(
            membuffer* m, const char* c_str) = 0;
    virtual int membuffer_append(
            membuffer* m, const void* buf, size_t buf_len) = 0;
    virtual int membuffer_append_str(
            membuffer* m, const char* c_str) = 0;
    virtual int membuffer_insert(
            membuffer* m, const void* buf, size_t buf_len, size_t index) = 0;
    virtual void membuffer_delete(
            membuffer* m, size_t index, size_t num_bytes) = 0;
    virtual char* membuffer_detach(
            membuffer* m) = 0;
    virtual void membuffer_attach(
            membuffer* m, char* new_buf, size_t buf_len) = 0;
};

class Cmembuffer : Imembuffer {
  public:
    virtual ~Cmembuffer() override {}

    char* str_alloc(const char* str, size_t str_len) override {
        return ::str_alloc(str, str_len); }
    int memptr_cmp(memptr* m, const char* s) override {
        return ::memptr_cmp(m, s); }
    int memptr_cmp_nocase(memptr* m, const char* s) override {
        return ::memptr_cmp_nocase(m, s); }
    int membuffer_set_size(membuffer* m, size_t new_length) override {
        return ::membuffer_set_size(m, new_length); }
    void membuffer_init(membuffer* m) override {
        return ::membuffer_init(m); }
    void membuffer_destroy(membuffer* m) override {
        return ::membuffer_destroy(m); }
    int membuffer_assign(membuffer* m, const void* buf, size_t buf_len) override {
        return ::membuffer_assign(m, buf, buf_len); }
    int membuffer_assign_str(membuffer* m, const char* c_str) override {
        return ::membuffer_assign_str(m, c_str); }
    int membuffer_append(membuffer* m, const void* buf, size_t buf_len) override {
        return ::membuffer_append(m, buf, buf_len); }
    int membuffer_append_str(membuffer* m, const char* c_str) override {
        return ::membuffer_append_str(m, c_str); }
    int membuffer_insert(membuffer* m, const void* buf, size_t buf_len, size_t index) override {
        return ::membuffer_insert(m, buf, buf_len, index); }
    void membuffer_delete(membuffer* m, size_t index, size_t num_bytes) override {
        return ::membuffer_delete(m, index, num_bytes); }
    char* membuffer_detach(membuffer* m) override {
        return ::membuffer_detach(m); }
    void membuffer_attach(membuffer* m, char* new_buf, size_t buf_len) override {
        return ::membuffer_attach(m, new_buf, buf_len); }
};
// clang-format on

//
TEST(MembufferTestSuite, init_and_destroy) {
    membuffer mbuf{};

    Cmembuffer mbObj{};
    mbObj.membuffer_init(&mbuf);
    EXPECT_EQ(mbuf.length, 0);
    EXPECT_EQ(mbuf.capacity, 0);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    ASSERT_EQ(mbuf.buf, nullptr);

    mbObj.membuffer_destroy(&mbuf);
    EXPECT_EQ(mbuf.length, 0);
    EXPECT_EQ(mbuf.capacity, 0);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    ASSERT_EQ(mbuf.buf, nullptr);
}

TEST(MembufferTestSuite, init_with_nullptr_to_membuf) {
#ifdef OLD_TEST
    ::std::cout << "  BUG! A nullptr to a membuffer must not segfault.\n";
#else
    Cmembuffer mbObj{};
    ASSERT_EXIT((mbObj.membuffer_init(nullptr), exit(0)),
                ::testing::ExitedWithCode(0), ".*")
        << "  # A nullptr to a membuffer must not segfault.";
#endif
}

TEST(MembufferTestSuite, destroy_with_nullptr_to_membuf) {
    Cmembuffer mbObj{};
    // destroying a nullptr should not segfault.
    ASSERT_EXIT((mbObj.membuffer_destroy(nullptr), exit(0)),
                ::testing::ExitedWithCode(0), ".*");
}

TEST(MembufferTestSuite, str_alloc_nullptr) {
    Cmembuffer mbObj{};
    char* strclone{};
    strclone = mbObj.str_alloc(nullptr, 0);
#ifdef OLD_TEST
    ::std::cout << "  BUG! A nullptr to a string with 0 size should never "
                   "allocate memory.\n";
    EXPECT_NE(strclone, nullptr);
    ::free(strclone);
#else
    EXPECT_EQ(strclone, nullptr) << "  # A nullptr to a string with 0 size "
                                    "should never allocate memory.";
#endif
}

TEST(MembufferTestSuite, str_alloc_nullptr_with_string_size) {
#ifdef OLD_TEST
    ::std::cout << "  BUG! A nullptr to a string but with given size must not "
                   "segfault.\n";
#else
    Cmembuffer mbObj{};
    char* strclone{};
    ASSERT_EXIT((strclone = mbObj.str_alloc(nullptr, 10), exit(0)),
                ::testing::ExitedWithCode(0), ".*")
        << "  # A nullptr to a string but with given size must not segfault.";
    EXPECT_EQ(strclone, nullptr);
#endif
}

TEST(MembufferTestSuite, str_alloc_empty_string_with_0_size) {
    Cmembuffer mbObj{};
    char* strclone{};
    strclone = mbObj.str_alloc("", 0);
    ASSERT_NE(strclone, nullptr);
    EXPECT_EQ(strclone[0], '\0');
    ::free(strclone);
}

TEST(MembufferTestSuite, str_alloc_valid_string_but_0_size_set) {
    Cmembuffer mbObj{};
    char* strclone{};
    strclone = mbObj.str_alloc("Hello World", 0);
    ASSERT_NE(strclone, nullptr);
    EXPECT_EQ(strclone[0], '\0');
    ::free(strclone);
}

TEST(MembufferTestSuite, str_alloc_valid_string_with_size) {
    Cmembuffer mbObj{};
    char* strclone{};
    strclone = mbObj.str_alloc("hello world!", 12);
    EXPECT_STREQ(strclone, "hello world!");
    ::free(strclone);
}

TEST(MembufferTestSuite, memptr_cmp_same_strings_varying_cases) {
    Cmembuffer mbObj{};
    char strbuf[32]{};
    ::memcpy(strbuf, "Test string", 12);
    memptr mbuf{strbuf, 12};
    int ret = mbObj.memptr_cmp(&mbuf, "Test string");
    EXPECT_EQ(ret, 0);

    // Different cases give different length. See man strcmp.
    // This returns < 0.
    ::memcpy(strbuf, "Test STRING", 12);
    ret = mbObj.memptr_cmp(&mbuf, "Test string");
    EXPECT_LT(ret, 0);

    // Same strings but swapped returns > 0.
    ::memcpy(strbuf, "Test string", 12);
    ret = mbObj.memptr_cmp(&mbuf, "Test STRING");
    EXPECT_GT(ret, 0);
}

TEST(MembufferTestSuite, memptr_cmp_different_strings) {
    Cmembuffer mbObj{};
    char strbuf[32]{};
    ::memcpy(strbuf, "Test-string", 12);
    memptr mbuf{strbuf, 12};
    int ret = mbObj.memptr_cmp(&mbuf, "Test string");
    EXPECT_GT(ret, 0);

    ::memcpy(strbuf, "Test string", 12);
    ret = mbObj.memptr_cmp(&mbuf, "Test-string");
    EXPECT_LT(ret, 0);

    ::memcpy(strbuf, "Test string", 12);
    ret = mbObj.memptr_cmp(&mbuf, "Test strings");
    EXPECT_LT(ret, 0);
}

TEST(MembufferTestSuite, memptr_cmp_with_empty_memory_pointer) {
    Cmembuffer mbObj{};
    memptr mbuf{};
    int ret = mbObj.memptr_cmp(&mbuf, "Test string");
    EXPECT_LT(ret, 0);
}

TEST(MembufferTestSuite, memptr_cmp_with_nullptr_to_mem_pointer) {
#ifdef OLD_TEST
    ::std::cout
        << "  BUG! A nullptr to the memory pointer must not segfault.\n";
#else
    Cmembuffer mbObj{};
    int ret{};
    ASSERT_EXIT((ret = mbObj.memptr_cmp(nullptr, "Test string"), exit(0)),
                ::testing::ExitedWithCode(0), ".*")
        << "  # A nullptr to the memory pointer must not segfault.";
    EXPECT_LT(ret, 0);
#endif
}

TEST(MembufferTestSuite, memptr_cmp_with_nullptr_to_compare_string) {
#ifdef OLD_TEST
    ::std::cout
        << "  BUG! A nullptr to the compared string must not segfault.\n";
#else
    Cmembuffer mbObj{};
    char strbuf[32]{};
    ::memcpy(strbuf, "Test string", 12);
    memptr mbuf{strbuf, 12};
    int ret{};
    ASSERT_EXIT((ret = mbObj.memptr_cmp(&mbuf, nullptr), exit(0)),
                ::testing::ExitedWithCode(0), ".*")
        << "  # A nullptr to the compared string must not segfault.";
    EXPECT_GT(ret, 0);
#endif
}

TEST(MembufferTestSuite, memptr_cmp_with_empty_compare_string) {
    Cmembuffer mbObj{};
    char strbuf[32]{};
    ::memcpy(strbuf, "Test string", 12);
    memptr mbuf{strbuf, 12};
    int ret = mbObj.memptr_cmp(&mbuf, "");
    EXPECT_GT(ret, 0);
}

TEST(MembufferTestSuite, memptr_cmp_nocase_same_strings_varying_cases) {
    Cmembuffer mbObj{};
    char strbuf[32]{};
    ::memcpy(strbuf, "Test string", 12);
    memptr mbuf{strbuf, 12};
    int ret = mbObj.memptr_cmp_nocase(&mbuf, "Test string");
    EXPECT_EQ(ret, 0);

    // No differences
    ::memcpy(strbuf, "Test STRING", 12);
    ret = mbObj.memptr_cmp_nocase(&mbuf, "Test string");
    EXPECT_EQ(ret, 0);

    // Also no differences
    ::memcpy(strbuf, "Test string", 12);
    ret = mbObj.memptr_cmp_nocase(&mbuf, "Test STRING");
    EXPECT_EQ(ret, 0);
}

TEST(MembufferTestSuite, memptr_cmp_nocase_different_strings) {
    Cmembuffer mbObj{};
    char strbuf[32]{};
    ::memcpy(strbuf, "Test-string", 12);
    memptr mbuf{strbuf, 12};
    int ret = mbObj.memptr_cmp_nocase(&mbuf, "Test string");
    EXPECT_GT(ret, 0);

    ::memcpy(strbuf, "Test string", 12);
    ret = mbObj.memptr_cmp_nocase(&mbuf, "Test-string");
    EXPECT_LT(ret, 0);

    ::memcpy(strbuf, "Test string", 12);
    ret = mbObj.memptr_cmp_nocase(&mbuf, "Test strings");
    EXPECT_LT(ret, 0);
}

TEST(MembufferTestSuite, memptr_cmp_nocase_with_empty_memory_pointer) {
    Cmembuffer mbObj{};
    // mbuf.buf is set to nullptr.
    memptr mbuf{};
    int ret{1};

#ifdef _WIN32
#ifdef OLD_TEST
    ::std::cout << "  BUG! A nullptr in memptr.buf of the memptr structure "
                   "must not crash on MS Windows.\n";
#else
    ASSERT_EXIT((ret = mbObj.memptr_cmp_nocase(&mbuf, "Test string"), exit(0)),
                ::testing::ExitedWithCode(0), ".*")
        << "  # A nullptr in memptr.buf of the memptr structure must not crash "
           "on MS Windows.";
    EXPECT_NE(ret, 0) << "  # A nullptr in memptr.buf of the memptr struct "
                         "should be invalid, not returning 0.";
#endif
#else // _WIN32
    ret = mbObj.memptr_cmp_nocase(&mbuf, "Test string");
#ifdef OLD_TEST
    ::std::cout << "  BUG! A nullptr in memptr.buf of the memptr struct should "
                   "be invalid, not returning -1.\n";
    EXPECT_EQ(ret, -1);
#else
    EXPECT_NE(ret, -1) << "  # A nullptr in memptr.buf of the memptr struct "
                          "should be invalid, not returning -1.";
#endif
#endif // _WIN32
}

TEST(MembufferTestSuite, memptr_cmp_nocase_with_nullptr_to_mem_pointer) {
#ifdef OLD_TEST
    ::std::cout
        << "  BUG! A nullptr to the memory pointer must not segfault.\n";
#else
    Cmembuffer mbObj{};
    int ret{};
    ASSERT_EXIT(
        (ret = mbObj.memptr_cmp_nocase(nullptr, "Test string"), exit(0)),
        ::testing::ExitedWithCode(0), ".*")
        << "  # A nullptr to the memory pointer must not segfault.";
    EXPECT_LT(ret, 0);
#endif
}

TEST(MembufferTestSuite, memptr_cmp_nocase_with_nullptr_to_compare_string) {
#ifdef OLD_TEST
    ::std::cout
        << "  BUG! A nullptr to the compared string must not segfault.\n";
#else
    Cmembuffer mbObj{};
    char strbuf[32]{};
    ::memcpy(strbuf, "Test string", 12);
    memptr mbuf{strbuf, 12};
    int ret{};
    ASSERT_EXIT((ret = mbObj.memptr_cmp_nocase(&mbuf, nullptr), exit(0)),
                ::testing::ExitedWithCode(0), ".*")
        << "  # A nullptr to the compared string must not segfault.";
    EXPECT_GT(ret, 0);
#endif
}

TEST(MembufferTestSuite, memptr_cmp_nocase_with_empty_compare_string) {
    Cmembuffer mbObj{};
    char strbuf[32]{};
    ::memcpy(strbuf, "Test string", 12);
    memptr mbuf{strbuf, 12};
    int ret = mbObj.memptr_cmp_nocase(&mbuf, "");
    EXPECT_GT(ret, 0);
}

TEST(MembufferTestSuite, membuffer_set_size_with_nullptr_to_membuf) {
#ifdef OLD_TEST
    ::std::cout << "  BUG! A nullptr to a membuffer must not segfault.\n";
#else
    Cmembuffer mbObj{};
    int ret{UPNP_E_INVALID_PARAM};
    ASSERT_EXIT((ret = mbObj.membuffer_set_size(nullptr, 1), exit(0)),
                ::testing::ExitedWithCode(0), ".*")
        << "  # A nullptr to a membuffer must not segfault.";
    EXPECT_EQ(ret, UPNP_E_OUTOF_MEMORY);
#endif
}

TEST(MembufferTestSuite, membuffer_set_size_to_0_byte_new_length) {
    membuffer mbuf{};

    Cmembuffer mbObj{};
    mbObj.membuffer_init(&mbuf);

    EXPECT_EQ(mbObj.membuffer_set_size(&mbuf, 0), UPNP_E_SUCCESS);
    EXPECT_EQ(mbuf.length, 0);
    EXPECT_EQ(mbuf.capacity, 0);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    EXPECT_EQ(mbuf.buf, nullptr);

    mbObj.membuffer_destroy(&mbuf);
}

TEST(MembufferTestSuite, membuffer_set_size_to_1_byte_new_length) {
    membuffer mbuf{};

    Cmembuffer mbObj{};
    mbObj.membuffer_init(&mbuf);

    mbObj.membuffer_set_size(&mbuf, 1);
    EXPECT_EQ(mbuf.length, 0);
    EXPECT_EQ(mbuf.capacity, 5);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    EXPECT_NE(mbuf.buf, nullptr);
    // Don't check strlen because the string isn't initialized and may have
    // varying strlen.
    // if (mbuf.buf != nullptr)
    //     EXPECT_EQ(strlen(mbuf.buf), 6);

    mbObj.membuffer_destroy(&mbuf);
}

TEST(MembufferTestSuite, membuffer_set_size) {
    membuffer mbuf{};

    Cmembuffer mbObj{};
    mbObj.membuffer_init(&mbuf);

    size_t new_length{2 * MEMBUF_DEF_SIZE_INC + 1};
    mbObj.membuffer_set_size(&mbuf, new_length);
    EXPECT_EQ(mbuf.length, 0);
    EXPECT_EQ(mbuf.capacity, new_length);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    EXPECT_NE(mbuf.buf, nullptr);
    // Don't check strlen because the string isn't initialized and may have
    // varying strlen.
    // if (mbuf.buf != nullptr)
    //     EXPECT_EQ(strlen(mbuf.buf), 6);

    mbObj.membuffer_destroy(&mbuf);
}

TEST(MembufferTestSuite, membuffer_assign) {
    membuffer mbuf{};

    Cmembuffer mbObj{};
    mbObj.membuffer_init(&mbuf);

    const unsigned char buf1[2]{1, 2};
    EXPECT_EQ(mbObj.membuffer_assign(&mbuf, &buf1, sizeof(buf1)),
              UPNP_E_SUCCESS);
    EXPECT_EQ(mbuf.length, 2);
    EXPECT_EQ(mbuf.capacity, 5);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    EXPECT_NE(mbuf.buf, nullptr);
    if (mbuf.buf) {
        EXPECT_EQ(strlen(mbuf.buf), 2);
        EXPECT_EQ(mbuf.buf[0], 1);
        EXPECT_EQ(mbuf.buf[1], 2);
        EXPECT_EQ(mbuf.buf[2], 0);
    }

    const unsigned char buf2[1]{3};
    EXPECT_EQ(mbObj.membuffer_assign(&mbuf, &buf2, sizeof(buf2)),
              UPNP_E_SUCCESS);
    EXPECT_EQ(mbuf.length, 1);
    EXPECT_EQ(mbuf.capacity, 5);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    EXPECT_NE(mbuf.buf, nullptr);
    if (mbuf.buf) {
        EXPECT_EQ(strlen(mbuf.buf), 1);
        EXPECT_EQ(mbuf.buf[0], 3);
        EXPECT_EQ(mbuf.buf[1], 0);
    }

    mbObj.membuffer_destroy(&mbuf);
}

TEST(MembufferTestSuite, membuffer_assign_with_nullptr_to_membufer) {
#ifdef OLD_TEST
    ::std::cout << "  BUG! A nullptr to a membuffer must not segfault.\n";
#else
    Cmembuffer mbObj{};
    const unsigned char buf[1]{0xFF};
    int ret{UPNP_E_INVALID_PARAM};
    ASSERT_EXIT(
        (ret = mbObj.membuffer_assign(nullptr, &buf, sizeof(buf)), exit(0)),
        ::testing::ExitedWithCode(0), ".*")
        << "  # A nullptr to a membuffer must not segfault.";
    EXPECT_EQ(ret, UPNP_E_OUTOF_MEMORY);
#endif
}

TEST(MembufferTestSuite, membuffer_assign_with_nullptr_to_compare_buffer) {
    membuffer mbuf{};

    Cmembuffer mbObj{};
    mbObj.membuffer_init(&mbuf);

    ASSERT_EQ(mbObj.membuffer_assign(&mbuf, nullptr, 10), UPNP_E_SUCCESS);
    EXPECT_EQ(mbuf.length, 0);
    EXPECT_EQ(mbuf.capacity, 0);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    EXPECT_EQ(mbuf.buf, nullptr);

    mbObj.membuffer_destroy(&mbuf);
}

TEST(MembufferTestSuite, membuffer_assign_with_0_byte_buffer_length) {
    membuffer mbuf{};
    const unsigned char buf[2]{1, 2};

    Cmembuffer mbObj{};
    mbObj.membuffer_init(&mbuf);

    EXPECT_EQ(mbObj.membuffer_assign(&mbuf, &buf, 0), UPNP_E_SUCCESS);
    EXPECT_EQ(mbuf.length, 0);
    EXPECT_EQ(mbuf.capacity, 0);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    EXPECT_EQ(mbuf.buf, nullptr);

    mbObj.membuffer_destroy(&mbuf);
}

TEST(MembufferTestSuite, membuffer_assign_str) {
    // membuffer_assign_str() just calls membuffer_assign() so that tests also
    // cover membuffer_assign_str().
    membuffer mbuf{};

    Cmembuffer mbObj{};
    mbObj.membuffer_init(&mbuf);

    const char str[]{"Hello World"};
    EXPECT_EQ(mbObj.membuffer_assign_str(&mbuf, str), UPNP_E_SUCCESS);
    EXPECT_EQ(mbuf.length, 11);
    EXPECT_EQ(mbuf.capacity, 11);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    EXPECT_NE(mbuf.buf, nullptr);
    if (mbuf.buf)
        EXPECT_STREQ(mbuf.buf, "Hello World");

    mbObj.membuffer_destroy(&mbuf);
}

TEST(MembufferTestSuite, membuffer_append) {
    // membuffer_append() just calls membuffer_insert() so that tests also
    // cover membuffer_append().
    membuffer mbuf{};

    Cmembuffer mbObj{};
    mbObj.membuffer_init(&mbuf);

    const char buf1[1]{1};
    EXPECT_EQ(mbObj.membuffer_append(&mbuf, &buf1, sizeof(buf1)), 0);
    EXPECT_EQ(mbuf.length, 1);
    EXPECT_EQ(mbuf.capacity, 5);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    EXPECT_NE(mbuf.buf, nullptr);
    if (mbuf.buf) {
        EXPECT_EQ(strlen(mbuf.buf), 1);
        EXPECT_EQ(mbuf.buf[0], 1);
        EXPECT_EQ(mbuf.buf[1], 0);
    }
    const char buf2[1]{2};
    EXPECT_EQ(mbObj.membuffer_append(&mbuf, &buf2, sizeof(buf2)), 0);
    EXPECT_EQ(mbuf.length, 2);
    EXPECT_EQ(mbuf.capacity, 5);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    EXPECT_NE(mbuf.buf, nullptr);
    if (mbuf.buf) {
        EXPECT_EQ(strlen(mbuf.buf), 2);
        EXPECT_EQ(mbuf.buf[0], 1);
        EXPECT_EQ(mbuf.buf[1], 2);
        EXPECT_EQ(mbuf.buf[2], 0);
    }
    const char buf3[4]{3, 4, 5, 6};
    EXPECT_EQ(mbObj.membuffer_append(&mbuf, &buf3, sizeof(buf3)), 0);
    EXPECT_EQ(mbuf.length, 6);
    EXPECT_EQ(mbuf.capacity, 10);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    EXPECT_NE(mbuf.buf, nullptr);
    if (mbuf.buf) {
        EXPECT_EQ(strlen(mbuf.buf), 6);
        EXPECT_EQ(mbuf.buf[0], 1);
        EXPECT_EQ(mbuf.buf[1], 2);
        EXPECT_EQ(mbuf.buf[2], 3);
        EXPECT_EQ(mbuf.buf[5], 6);
        EXPECT_EQ(mbuf.buf[6], 0);
    }
    mbObj.membuffer_destroy(&mbuf);
}

TEST(MembufferTestSuite, membuffer_append_str) {
    // membuffer_append_str() just calls membuffer_insert() so that tests also
    // cover membuffer_append_str().
    membuffer mbuf{};

    Cmembuffer mbObj{};
    mbObj.membuffer_init(&mbuf);

    const char buf1[]{"Hello"};
    EXPECT_EQ(mbObj.membuffer_append_str(&mbuf, buf1), 0);
    EXPECT_EQ(mbuf.length, 5);
    EXPECT_EQ(mbuf.capacity, 5);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    EXPECT_NE(mbuf.buf, nullptr);
    if (mbuf.buf) {
        EXPECT_EQ(strlen(mbuf.buf), 5);
        EXPECT_STREQ(mbuf.buf, "Hello");
    }
    const char buf2[]{" World"};
    EXPECT_EQ(mbObj.membuffer_append_str(&mbuf, buf2), 0);
    EXPECT_EQ(mbuf.length, 11);
    EXPECT_EQ(mbuf.capacity, 11);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    EXPECT_NE(mbuf.buf, nullptr);
    if (mbuf.buf) {
        EXPECT_EQ(strlen(mbuf.buf), 11);
        EXPECT_STREQ(mbuf.buf, "Hello World");
    }
    mbObj.membuffer_destroy(&mbuf);
}

TEST(MembufferTestSuite, membuffer_insert) {
    // For other successful tests see tests to membuffer_append and
    // membuffer_append_str.
    membuffer mbuf{};

    Cmembuffer mbObj{};
    mbObj.membuffer_init(&mbuf);

    const char buf[1]{1};
    EXPECT_EQ(mbObj.membuffer_insert(&mbuf, &buf, 1, mbuf.length), 0);
    EXPECT_EQ(mbuf.length, 1);
    EXPECT_EQ(mbuf.capacity, 5);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    EXPECT_NE(mbuf.buf, nullptr);
    if (mbuf.buf) {
        EXPECT_EQ(strlen(mbuf.buf), 1);
        EXPECT_EQ(mbuf.buf[0], 1);
        EXPECT_EQ(mbuf.buf[1], 0);
    }
    mbObj.membuffer_destroy(&mbuf);
}

TEST(MembufferTestSuite, membuffer_insert_with_nullptr_to_membuf) {
    // For successful tests see tests to membuffer_append and
    // membuffer_append_str.
#ifdef OLD_TEST
    ::std::cout << "  BUG! A nullptr to a membuffer must not segfault.\n";
#else
    Cmembuffer mbObj{};
    const char buf[1]{1};
    int ret{UPNP_E_INVALID_PARAM};
    ASSERT_EXIT(
        (ret = mbObj.membuffer_insert(nullptr, &buf, sizeof(buf), 0), exit(0)),
        ::testing::ExitedWithCode(0), ".*")
        << "  # A nullptr to a membuffer must not segfault.";
    EXPECT_EQ(ret, UPNP_E_OUTOF_MEMORY);
#endif
}

TEST(MembufferTestSuite, membuffer_insert_with_nullptr_to_source_buffer) {
    // For other successful tests see tests to membuffer_append and
    // membuffer_append_str.
    membuffer mbuf{};

    Cmembuffer mbObj{};
    mbObj.membuffer_init(&mbuf);

    EXPECT_EQ(mbObj.membuffer_insert(&mbuf, nullptr, 0, 0), 0);
    EXPECT_EQ(mbuf.length, 0);
    EXPECT_EQ(mbuf.capacity, 0);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    EXPECT_EQ(mbuf.buf, nullptr);

    mbObj.membuffer_destroy(&mbuf);
}

TEST(MembufferTestSuite, membuffer_insert_with_empty_source_buffer) {
    // For other successful tests see tests to membuffer_append and
    // membuffer_append_str.
    membuffer mbuf{};

    Cmembuffer mbObj{};
    mbObj.membuffer_init(&mbuf);

    const char buf[1]{""};
    EXPECT_EQ(mbObj.membuffer_insert(&mbuf, &buf, 0, 0), 0);
    EXPECT_EQ(mbuf.length, 0);
    EXPECT_EQ(mbuf.capacity, 0);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    EXPECT_EQ(mbuf.buf, nullptr);

    mbObj.membuffer_destroy(&mbuf);
}

TEST(MembufferTestSuite, membuffer_insert_with_0_length) {
    // For other successful tests see tests to membuffer_append and
    // membuffer_append_str.
    membuffer mbuf{};

    Cmembuffer mbObj{};
    mbObj.membuffer_init(&mbuf);

    union Buf {
        const char filler[2];
        const char buf[1];
    };
    Buf b = {1, '\xFF'};

    // b.buf has only one char.
    EXPECT_EQ(mbObj.membuffer_insert(&mbuf, &b.buf, sizeof(b.buf) + 1, 0), 0);
    EXPECT_EQ(mbuf.length, 2);
    EXPECT_EQ(mbuf.capacity, 5);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    EXPECT_NE(mbuf.buf, nullptr);
    if (mbuf.buf) {
        EXPECT_EQ(strlen(mbuf.buf), 2);
        EXPECT_EQ(mbuf.buf[0], 1);
#ifdef OLD_TEST
        ::std::cout << "  BUG! Length > sizeof source buffer should not give "
                       "uninitialized content.\n";
        EXPECT_EQ(mbuf.buf[1], '\xFF');
#else
        EXPECT_EQ(mbuf.buf[1], 0)
            << "  # Length > sizeof source buffer should not give "
               "uninitialized content.";
#endif
        EXPECT_EQ(mbuf.buf[2], 0);
    }
    mbObj.membuffer_destroy(&mbuf);
}

TEST(MembufferTestSuite, membuffer_insert_with_additional_capacity) {
    // For other successful tests see tests to membuffer_append and
    // membuffer_append_str.
    membuffer mbuf{};

    Cmembuffer mbObj{};
    mbObj.membuffer_init(&mbuf);

    const char buf[1]{1};
    EXPECT_EQ(mbObj.membuffer_insert(&mbuf, &buf, sizeof(buf), 1),
              UPNP_E_OUTOF_BOUNDS);
    EXPECT_EQ(mbuf.length, 0);
    EXPECT_EQ(mbuf.capacity, 0);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    EXPECT_EQ(mbuf.buf, nullptr);

    mbObj.membuffer_destroy(&mbuf);
}

TEST(MembufferTestSuite, membuffer_delete) {
    membuffer mbuf{};

    Cmembuffer mbObj{};
    mbObj.membuffer_init(&mbuf);

    const char buf[]{"Hello World"};
    EXPECT_EQ(mbObj.membuffer_insert(&mbuf, &buf, sizeof(buf), mbuf.length), 0);
    EXPECT_EQ(mbuf.length, 12);
    EXPECT_EQ(mbuf.capacity, 12);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    ASSERT_NE(mbuf.buf, nullptr);
    if (mbuf.buf) {
        EXPECT_EQ(strlen(mbuf.buf), 11);
        EXPECT_STREQ(mbuf.buf, "Hello World");
    }

    // void membuffer_delete(membuffer* mbuf, size_t index, size_t num_bytes)
    // num_bytes are the bytes to delete beginning at index + 1.
    mbObj.membuffer_delete(&mbuf, 5, 6);
    EXPECT_EQ(mbuf.length, 6);
    EXPECT_EQ(mbuf.capacity, 11);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    ASSERT_NE(mbuf.buf, nullptr);
    if (mbuf.buf) {
        EXPECT_EQ(strlen(mbuf.buf), 5);
        EXPECT_STREQ(mbuf.buf, "Hello");
    }
    mbObj.membuffer_delete(&mbuf, 4, 6);
    EXPECT_EQ(mbuf.length, 4);
    EXPECT_EQ(mbuf.capacity, 9);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    ASSERT_NE(mbuf.buf, nullptr);
    if (mbuf.buf) {
        EXPECT_EQ(strlen(mbuf.buf), 4);
        EXPECT_STREQ(mbuf.buf, "Hell");
    }
    mbObj.membuffer_delete(&mbuf, 0, 0);
    EXPECT_EQ(mbuf.length, 4);
    EXPECT_EQ(mbuf.capacity, 9);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    ASSERT_NE(mbuf.buf, nullptr);
    if (mbuf.buf) {
        EXPECT_EQ(strlen(mbuf.buf), 4);
        EXPECT_STREQ(mbuf.buf, "Hell");
    }
    mbObj.membuffer_delete(&mbuf, 1, 0);
    EXPECT_EQ(mbuf.length, 4);
    EXPECT_EQ(mbuf.capacity, 9);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    ASSERT_NE(mbuf.buf, nullptr);
    if (mbuf.buf) {
        EXPECT_EQ(strlen(mbuf.buf), 4);
        EXPECT_STREQ(mbuf.buf, "Hell");
    }
    mbObj.membuffer_delete(&mbuf, 0, 1);
    EXPECT_EQ(mbuf.length, 3);
    EXPECT_EQ(mbuf.capacity, 8);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    ASSERT_NE(mbuf.buf, nullptr);
    if (mbuf.buf) {
        EXPECT_EQ(strlen(mbuf.buf), 3);
        EXPECT_STREQ(mbuf.buf, "ell");
    }
    // This will empty the string in memory buffer
    mbObj.membuffer_delete(&mbuf, 0, 100);
    EXPECT_EQ(mbuf.length, 0);
    EXPECT_EQ(mbuf.capacity, 5);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    ASSERT_NE(mbuf.buf, nullptr);
    if (mbuf.buf) {
        EXPECT_EQ(strlen(mbuf.buf), 0);
        EXPECT_STREQ(mbuf.buf, "");
    }

    mbObj.membuffer_destroy(&mbuf);

    ASSERT_EXIT((mbObj.membuffer_delete(nullptr, 0, 100), exit(0)),
                ::testing::ExitedWithCode(0), ".*")
        << "  # A nullptr to a membuffer must not segfault.";
}

TEST(MembufferTestSuite, membuffer_detach) {
    membuffer mbuf{};

    Cmembuffer mbObj{};
    mbObj.membuffer_init(&mbuf);

    const char buf[]{"Hello World"};
    EXPECT_EQ(mbObj.membuffer_insert(&mbuf, &buf, sizeof(buf), mbuf.length), 0);

    char* detached = mbObj.membuffer_detach(&mbuf);
    EXPECT_STREQ(detached, "Hello World");
    ::free(detached);

    EXPECT_EQ(mbuf.length, 0);
    EXPECT_EQ(mbuf.capacity, 0);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    ASSERT_EQ(mbuf.buf, nullptr);

    mbObj.membuffer_destroy(&mbuf);
}

TEST(MembufferTestSuite, membuffer_detach_with_nullptr_to_membuffer) {
#ifdef OLD_TEST
    ::std::cout << "  BUG! A nullptr to a membuffer must not segfault.\n";
#else
    Cmembuffer mbObj{};
    char* detached{nullptr};
    ASSERT_EXIT((detached = mbObj.membuffer_detach(nullptr), exit(0)),
                ::testing::ExitedWithCode(0), ".*")
        << "  # A nullptr to a membuffer must not segfault.";
    EXPECT_EQ(detached, nullptr);
#endif
}

TEST(MembufferTestSuite, membuffer_attach) {
    membuffer mbuf{};

    Cmembuffer mbObj{};
    mbObj.membuffer_init(&mbuf);

    const char old_buf[]{"Old buf"};
    EXPECT_EQ(mbObj.membuffer_append_str(&mbuf, old_buf), 0);
    ASSERT_NE(mbuf.buf, nullptr);
    if (mbuf.buf)
        EXPECT_STREQ(mbuf.buf, "Old buf");

    char* new_buf = mbObj.str_alloc("Hello World", 11);
    mbObj.membuffer_attach(&mbuf, new_buf, 11);
    EXPECT_EQ(mbuf.length, 11);
    EXPECT_EQ(mbuf.capacity, 11);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    ASSERT_NE(mbuf.buf, nullptr);
    if (mbuf.buf) {
        EXPECT_EQ(strlen(mbuf.buf), 11);
        EXPECT_STREQ(mbuf.buf, "Hello World");
    }
    mbObj.membuffer_destroy(&mbuf);
}

TEST(MembufferTestSuite, membuffer_attach_with_nullptr_to_membuffer) {
#ifdef OLD_TEST
    ::std::cout << "  BUG! A nullptr to a membuffer must not segfault.\n";
#else
    Cmembuffer mbObj{};
    char* new_buf = mbObj.str_alloc("Hello World", 11);
    EXPECT_EXIT((mbObj.membuffer_attach(nullptr, new_buf, 11), exit(0)),
                ::testing::ExitedWithCode(0), ".*")
        << "  # A nullptr to a membuffer must not segfault.";
    ::free(new_buf);
#endif
}

TEST(MembufferTestSuite, membuffer_attach_with_nullptr_to_string_buffer) {
    membuffer mbuf{};

    Cmembuffer mbObj{};
    mbObj.membuffer_init(&mbuf);

    // Create membuffer with a string
    const char old_buf[]{"Old buf"};
    EXPECT_EQ(mbObj.membuffer_append_str(&mbuf, old_buf), 0);
    EXPECT_EQ(mbuf.length, 7);
    EXPECT_EQ(mbuf.capacity, 7);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    ASSERT_NE(mbuf.buf, nullptr);
    if (mbuf.buf)
        EXPECT_STREQ(mbuf.buf, "Old buf");

    // Try to attach a nullptr
    mbObj.membuffer_attach(&mbuf, nullptr, 11);
#ifdef OLD_TEST
    ::std::cout << "  BUG! A nullptr to a buffer that should be attached must "
                   "not segfault.\n";
    EXPECT_EQ(mbuf.length, 11);
    EXPECT_EQ(mbuf.capacity, 11);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    ASSERT_EQ(mbuf.buf, nullptr);
#else
    EXPECT_EQ(mbuf.length, 7) << "  # A nullptr to a buffer that should be "
                                 "attached must not segfault.";
    EXPECT_EQ(mbuf.capacity, 7);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    ASSERT_NE(mbuf.buf, nullptr);
    if (mbuf.buf)
        EXPECT_STREQ(mbuf.buf, "Old buf");
#endif
    mbObj.membuffer_destroy(&mbuf);
}

TEST(MembufferTestSuite, membuffer_attach_with_empty_string_buffer) {
    membuffer mbuf{};

    Cmembuffer mbObj{};
    mbObj.membuffer_init(&mbuf);

    // Create membuffer with a string
    const char old_buf[]{"Old buf"};
    EXPECT_EQ(mbObj.membuffer_append_str(&mbuf, old_buf), 0);
    EXPECT_EQ(mbuf.length, 7);
    EXPECT_EQ(mbuf.capacity, 7);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    ASSERT_NE(mbuf.buf, nullptr);
    if (mbuf.buf)
        EXPECT_STREQ(mbuf.buf, "Old buf");

    // Attach empty string to the membuffer
    char* new_buf = mbObj.str_alloc("", 0);
    mbObj.membuffer_attach(&mbuf, new_buf, 0);
    EXPECT_EQ(mbuf.length, 0);
    EXPECT_EQ(mbuf.capacity, 0);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    ASSERT_NE(mbuf.buf, nullptr);
    if (mbuf.buf)
        EXPECT_EQ(strlen(mbuf.buf), 0);
    EXPECT_STREQ(mbuf.buf, "");

    mbObj.membuffer_destroy(&mbuf);
}

TEST(MembufferTestSuite,
     membuffer_attach_with_empty_string_buffer_but_buffer_length) {
    membuffer mbuf{};

    Cmembuffer mbObj{};
    mbObj.membuffer_init(&mbuf);

    // Create membuffer with a string
    const char old_buf[]{"Old buf"};
    EXPECT_EQ(mbObj.membuffer_append_str(&mbuf, old_buf), 0);
    EXPECT_EQ(mbuf.length, 7);
    EXPECT_EQ(mbuf.capacity, 7);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    ASSERT_NE(mbuf.buf, nullptr);
    if (mbuf.buf)
        EXPECT_STREQ(mbuf.buf, "Old buf");

    char* new_buf = mbObj.str_alloc("", 0);
    // Attach empty string but with buffer length
    mbObj.membuffer_attach(&mbuf, new_buf, 11);
    EXPECT_EQ(mbuf.length, 11);
    EXPECT_EQ(mbuf.capacity, 11);
    EXPECT_EQ(mbuf.size_inc, MEMBUF_DEF_SIZE_INC);
    ASSERT_NE(mbuf.buf, nullptr);
    if (mbuf.buf)
        EXPECT_EQ(strlen(mbuf.buf), 0);
    EXPECT_STREQ(mbuf.buf, "");

    mbObj.membuffer_destroy(&mbuf);
}

} // namespace upnp

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    // ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
