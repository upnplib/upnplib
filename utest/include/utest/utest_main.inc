// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-10-25

// Because this part of the main entry function uses gtest macros we have to
// include it directly into the source of the test. These macros need to have
// direct access to test macros.
//
// There is a last old version with parameter for the compiled test to run it
// with old or new code at commit 562ee325c563f316498367fce64410a5a1ef15f6.

// Parse for upnplib arguments prefixed with '--upnplib'. InitGoogleTest()
// has removed its options prefixed with '--gtest' from the arguments and
// corrected argc accordingly.

int gtest_return_code{EXIT_FAILURE};

std::string_view usage_hint{
    "Invalid argument. Valid upnplib option is:\n--upnplib_debug - to enable "
    "debug messages\n"};

if (argc > 2) {
    std::cerr << usage_hint;
    goto finish;
}

if (argc == 2) {
    if ((strncmp(argv[1], "--upnplib_debug", 15) == 0) &&
        (strlen(argv[1]) == 15)) {
        upnplib::g_dbug = true;
    } else {
        std::cerr << usage_hint;
        goto finish;
    }
}

// If the debug flag wasn't set with the commandline option then check if it
// shall be set with the environment variable.
if (!upnplib::g_dbug)
    upnplib::g_dbug = static_cast<bool>(std::getenv("UPNPLIB_DEBUG"));

gtest_return_code = RUN_ALL_TESTS();
#ifdef UPNPLIB_WITH_NATIVE_PUPNP
std::cout << "[----------] Tested UPnPlib old code.\n\n";
#else
std::cout << "[----------] Tested UPnPlib new code.\n\n";
#endif
finish:

    // vim: syntax=cpp
