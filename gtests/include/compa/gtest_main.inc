// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-03-08

// Because this part of the main entry function uses gtest macros we have to
// include it directly into the source of the test. These macros need to have
// direct access to test macros.
//
// Parse for pupnp arguments prefixed with '--pupnp'. InitGoogleTest()
// has removed its options prefixed with '--gtest' from the arguments and
// corrected argc accordingly.

int gtest_return_code{EXIT_FAILURE};

if (argc > 2) {
    std::cerr << "Too many arguments supplied. Valid pupnp options "
                 "only:\n--pupnp_old_code\n--pupnp_new_code"
              << std::endl;

} else {

    bool both{false};
    if (argc == 2) {
        if ((strncmp(argv[1], "--pupnp_old_code", 16) == 0) &&
            (strlen(argv[1]) == 16)) {
            compa::old_code = true;
        } else if ((strncmp(argv[1], "--pupnp_new_code", 16) == 0) &&
                   (strlen(argv[1]) == 16)) {
            compa::old_code = false;
        } else {
            std::cerr << "Unknown argument. Valid pupnp options "
                         "only:\n--pupnp_old_code\n--pupnp_new_code"
                      << std::endl;
        }
    } else { // There is no --pupnp* argument
             // both = true;
#ifdef UPNPLIB_WITH_NATIVE_PUPNP
        compa::old_code = true;
#else
        compa::old_code = false;
#endif
    }

    if (both) {
        compa::old_code = true;
        int rc1 = RUN_ALL_TESTS();
        std::cout << "[----------] Tested pupnp old code.\n\n";

        compa::old_code = false;
        int rc2 = RUN_ALL_TESTS();
        std::cout << "[----------] Tested pupnp new code.\n";

        if (rc1)
            ::std::cout << "[  FAILED  ] Tests for pupnp old code.\n";
        else
            ::std::cout << "[  PASSED  ] Tests for pupnp old code.\n";
        if (rc2)
            ::std::cout << "[  FAILED  ] Tests for pupnp new code.\n";
        else
            ::std::cout << "[  PASSED  ] Tests for pupnp new code.\n";
        ::std::cout << ::std::endl;

        gtest_return_code = rc1 | rc2; // Binary OR is intended

    } else if (compa::old_code) {
        gtest_return_code = RUN_ALL_TESTS();
        std::cout << "[----------] Tested pupnp old code.\n\n";

    } else {
        gtest_return_code = RUN_ALL_TESTS();
        std::cout << "[----------] Tested pupnp new code.\n\n";
    }

} // (argc > 2)

// vim: syntax=cpp
