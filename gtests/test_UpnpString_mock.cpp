// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-10-02

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::_;
using ::testing::Eq;
using ::testing::Return;

// --- mock calloc ---------------------------------------
class CMock_calloc {
  public:
    MOCK_METHOD(void*, calloc, (size_t nmemb, size_t size));
};
CMock_calloc* ptrMock_calloc = nullptr;

// --- mock free -----------------------------------------
class CMock_free {
  public:
    MOCK_METHOD(void, free, (void* ptr));
};
CMock_free* ptrMock_free = nullptr;

namespace { // no name, i.e. anonymous for file scope
            // this is the C++ way for decorator STATIC

void* calloc(size_t nmemb, size_t size) {
    return ptrMock_calloc->calloc(nmemb, size);
}
void free(void* ptr) { return ptrMock_free->free(ptr); }

#include "api/UpnpString.cpp"

} // namespace

// testsuite with fixtures
//------------------------
class UpnpStringMockTestSuite : public ::testing::Test {
  protected:
    // Instantiate the mock objects.
    // The global pointer to them are set in the constructor below.
    CMock_calloc mock_calloc;
    CMock_free mock_free;

    UpnpStringMockTestSuite() {
        // set the global pointer to the mock objects
        ptrMock_calloc = &mock_calloc;
        ptrMock_free = &mock_free;
    }
};

TEST_F(UpnpStringMockTestSuite, createNewUpnpString) {
    // provide a structure of a UpnpString
    char mstring[] = {0};
    SUpnpString upnpstr = {};
    UpnpString* p = (UpnpString*)&upnpstr;
    UpnpString* str;

    EXPECT_CALL(mock_calloc, calloc(_, _))
        .WillOnce(Return(p))
        .WillOnce(Return(*&mstring));
    // call the unit
    str = UpnpString_new();
    EXPECT_THAT(str, Eq(p));

    // test edge conditions
    EXPECT_CALL(mock_calloc, calloc(_, _)).WillOnce(Return((UpnpString*)NULL));
    // call the unit
    str = UpnpString_new();
    EXPECT_EQ(str, (UpnpString*)NULL);

    EXPECT_CALL(mock_calloc, calloc(_, _))
        .WillOnce(Return(p))
        .WillOnce(Return((char*)NULL));
    EXPECT_CALL(mock_free, free(_)).Times(1);
    // call the unit
    str = UpnpString_new();
    EXPECT_EQ(str, (UpnpString*)NULL);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
