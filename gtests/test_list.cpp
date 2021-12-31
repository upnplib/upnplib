// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-12-31

#include "gtest/gtest.h"
//#include "gmock/gmock.h"

#include "genlib/util/list.cpp"

namespace upnp {

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
TEST(ListTestSuite, UpnpListInit) {}

} // namespace upnp

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    // ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
