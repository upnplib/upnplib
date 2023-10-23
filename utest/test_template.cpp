// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-10-24

// This test should always run, reporting no failure

#include <utest/utest.hpp>
// #include <gtest/gtest.h>
#include <gmock/gmock.h> // preferred because it contains also gtest.h

// two macros to compare values in a range
#define EXPECT_IN_RANGE(VAL, MIN, MAX)                                         \
    EXPECT_GE((VAL), (MIN));                                                   \
    EXPECT_LE((VAL), (MAX))

#define ASSERT_IN_RANGE(VAL, MIN, MAX)                                         \
    ASSERT_GE((VAL), (MIN));                                                   \
    ASSERT_LE((VAL), (MAX))

// ANSI console colors
#define CRED "\033[38;5;203m" // red
#define CYEL "\033[38;5;227m" // yellow
#define CGRN "\033[38;5;83m"  // green
#define CRES "\033[0m"        // reset

namespace utest {

// testsuite with fixtures
//------------------------
class EmptyFixtureTestSuite : public ::testing::Test {
    // You can remove any or all of the following functions if their bodies
    // would be empty.

  public:
    static void SetUpTestSuite() {
        // Here you can do set-up work once for all tests on the TestSuite.
    }

    static void TearDownTestSuite() {
        // Do clean-up work once after the last test of the TestSuite
        // that doesn't throw exceptions here.
    }

  protected:
    // Instantiate the mock objects.
    // The global pointer to them are set in the constructor below.
    // CMock_strerror mock_strerror;

    EmptyFixtureTestSuite() {
        // Constructor, you can do set-up work for each test here.
        // In the body of a constructor (or destructor), it’s not possible to
        // use the ASSERT_xx macros. More details at
        // https://google.github.io/googletest/faq.html#CtorVsSetUp

        // set the global pointer to the mock objects
        // ptrMock_strerror = &mock_strerror;
    }

    ~EmptyFixtureTestSuite() override {
        // Destructor, do clean-up work that doesn't throw exceptions here.
        // Marking destructors override on state-owning subclasses is textbook
        // hygiene that you should all be doing by routine.
    }

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following methods:

    void SetUp() override {
        // Code here will be called immediately after the constructor (right
        // before each test). Have attention to the uppercase 'U' of SetUp().
    }

    void TearDown() override {
        // Code here will be called immediately after each test (right
        // before the destructor).
    }

    // Class members declared here can be used by all tests in the test suite
    // for Foo.
};

TEST_F(EmptyFixtureTestSuite, empty_gtest_with_fixture) {}

// simple testsuite without fixtures
//----------------------------------
TEST(EmptyTestSuite, skip_on_github_action) {
    if (github_actions)
        GTEST_SKIP() << "Test is skipped on Github Actions.";
}

TEST(EmptyTestSuite, empty_gtest) {
    // SKIP on Github Actions
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // GTEST_SKIP_("to show this feature");
    // GTEST_SKIP() << "to show this feature\n";

    if (old_code) {
        // Place tests for old_code here
        std::cout << CYEL "[ BUG      ]" CRES
                  << " This old_code bug must be fixed in new_code.\n";
        std::cout << CYEL "[ BUGFIX   ]" CRES
                  << " This old_code bug is fixed in new_code.\n";
        // or
        std::cout << CYEL "[ OPT      ]" CRES
                  << " Optimization or Option needed to improve the program.\n";
    } else {
        // Place tests for new_code here
        std::cout << "  # Compiling tests for new_code.\n";
    }
}

} // namespace utest


int main(int argc, char** argv) {
    // ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv); // preferred
    return RUN_ALL_TESTS();                 // or next line
    // #include <utest/utest_main.inc>
    // return gtest_return_code; // managed in utest/utest_main.inc
}
