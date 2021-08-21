## Use shared libraries
By default Googletest builds static libraries and suggest to use them to be portable. On the other side Googletest strongly recommend to build its libraries together with the project to have the same environment and chaintools and avoid possible compatibillity problems. Having Googletest as part of the project you can run the Unit-Tests only on the installed project. So you can use its shared libraries without additional limitations because they point also to the installed project. Here is a compare of file sizes from 5 Unit Tests:

###linked with static gtest/gmock libraries
    -rwxr-xr-x 1 ingo ingo  957704 2021-08-21 16:24 build/test_template
    -rwxr-xr-x 1 ingo ingo 1138496 2021-08-21 16:24 build/test_threadutil
    -rwxr-xr-x 1 ingo ingo 1672400 2021-08-21 16:24 build/test_upnpapi
    -rwxr-xr-x 1 ingo ingo 1419864 2021-08-21 16:24 build/test_upnpdebug
    -rwxr-xr-x 1 ingo ingo 1002416 2021-08-21 16:23 build/test_upnpdebug_nomock

###linked with shared gtest/gmock libraries
    -rwxr-xr-x 1 ingo ingo  26936 2021-08-22 02:10 build/test_template
    -rwxr-xr-x 1 ingo ingo 205136 2021-08-22 02:10 build/test_threadutil
    -rwxr-xr-x 1 ingo ingo 504832 2021-08-22 02:10 build/test_upnpapi
    -rwxr-xr-x 1 ingo ingo 248144 2021-08-22 02:10 build/test_upnpdebug
    -rwxr-xr-x 1 ingo ingo  92536 2021-08-22 02:10 build/test_upnpdebug_nomock

Each file is expanded with about 1 MB of the static Googletest code. Because I plan to use a lot of Unit Tests sharing the Googletest code seems to be a good idea.

## Compile and run tests
If the output is messed up with logging information from the library,
redirect stderr to /dev/null:

    ./build/test_upnpapi.a 2>/dev/null

## Skip tests on Github Workflow Actions
It is possible that you have tests which are failing because fixing the bug it
is showing takes some time. To be able to push bug fixes and create pull
requests you can skip tests if they run on the Github Workflow as Action. The
test checks if the environment variable `GITHUB_ACTIONS` exists and will then
skip the test. An example for this conditional check you can find in the
test_template.cpp. If you want to see what tests are skipped on Github Actions
you can execute the test for example with:

    GITHUB_ACTIONS="true" ./build/test_template.a

<br /><pre>
// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  &#60;Ingo&#64;Hoeft-online.de&#62;
// Redistribution only with this Copyright remark. Last modified: 2021-08-22
</pre>
