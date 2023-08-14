## Use shared libraries
By default Googletest builds static libraries and suggest to use them to be
portable. On the other side Googletest strongly recommend to build its
libraries together with the project to have the same environment and chaintools
and avoid possible compatibillity problems. Having Googletest as part of the
project you can run the Unit-Tests only on the installed project. So you can
use its shared libraries without additional limitations because they point also
to the installed project. Here is a compare of file sizes from 5 Unit Tests:

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

Each file is expanded with about 1 MB of the static Googletest code. Because I
plan to use a lot of Unit Tests sharing the Googletest code seems to be a good
idea.

## Skip tests on Github Workflow Actions
It is possible that you have tests which are failing because fixing the bug it
is showing takes some time. To be able to push bug fixes and create pull
requests you can skip tests if they run on the Github Workflow as Action. The
test checks if the environment variable `GITHUB_ACTIONS` exists and will then
skip the test. An example for this conditional check you can find in the
test_template.cpp. If you want to see what tests are skipped on Github Actions
you can execute the test for example with:

    ~$ GITHUB_ACTIONS=true ./build/gtests/test_template

To run all tests local from the projects root directory with GITHUB_ACTIONS I use:

    ~$ (cd ./build && GITHUB_ACTIONS=true ctest --timeout 2 --output-on-failure)

## Find different test variables
To avoid conflicts on different tests there are some varibles that have to be
unique over all tests.

Due to 'man select' socket file descriptors must be less than FD_SETSIZE (1024)
to be supported. To have unique socket file descriptors I use 'FD_SETSIZE -
[1,2,..]' and search for used fds for example with:

    ~$ grep -Pnor --color=never --include='*.[chi]*' 'FD_SETSIZE - \d+' ./gtests | sort -t: -k3.14n
    ./gtests/sample/test_tv_device.cpp:165:FD_SETSIZE - 1
    ./gtests/sample/test_tv_device.cpp:184:FD_SETSIZE - 2
    ./gtests/compa/test_miniserver.cpp:1673:FD_SETSIZE - 3
    ./gtests/sample/test_tv_device.cpp:207:FD_SETSIZE - 3
    ./gtests/compa/test_miniserver.cpp:1674:FD_SETSIZE - 5
    ./gtests/compa/test_miniserver.cpp:1676:FD_SETSIZE - 6

There is a problem with `FD_SETSIZE - 3` and should be fixed. `FD_SETSIZE - 4`
can be used.

It is also strongly recommended to use unique port numbers for testing to avoid
getting a delay to re-use an ip address. I start with test port number 50000
and use a simple search that does not necessarily find only used port nummbers.
But it doesn't matter as long as the new used number isn't found. I use this:

    ~$ grep -Phor --include='*.[chi]*' '5\d\d\d\d' ./gtests | sort -n | uniq

<br /><pre>
// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  &#60;Ingo&#64;Hoeft-online.de&#62;
// Redistribution only with this Copyright remark. Last modified: 2023-08-13
</pre>
