// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-12-23

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

} // namespace upnp

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    // ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
