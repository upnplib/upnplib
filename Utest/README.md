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

    ~$ GITHUB_ACTIONS=true ./build/Utest/test_template

To run all tests local from the projects root directory with GITHUB_ACTIONS I use:

    ~$ (cd ./build && GITHUB_ACTIONS=true ctest --timeout 2 --output-on-failure)

## Find different unique values
There are some values that must be unique system wide. To find all used values
here are some pattern using grep.

All error messages have a unique nummber so the number does not only specify the associated message text but also its location in the source code. This also means that messages with the same text but a different location in the source have different numbers. Or with other words, each message have a unique index number, no matter what it is meaning. By default the message numbers start with 1000. Numbers below are reservered for special use. You can find already used message numbers with:

    ~$ # Get used error message numbers
    ~$ grep -Pnor --color=never --exclude-dir={build,html,} --include='*.[chi]*' '...MSG\d\d\d\d...' | sort -t: -k3.4

To avoid conflicts with double used socket file descriptors (sfd) on tests I
always use a new one. To have a simple search pattern I define a constant
`umock::sfd_base` and set a new socket file descriptor for testing for example
with 'umock::sfd_base + 1' so I can simply grep for 'sfd_base' to find already
used ones.

IMPORTANT! There is a limit FD_SETSIZE = 1024 for socket file descriptors
that can be used with 'select()'. We must not use more than 1023 fds.
Otherwise we have undefined behavior and may get segfaults with 'FD_SET()'.
For details have a look at 'man select'.

To find all used file descriptors in tests I use this command:

    ~$ # Get used file descriptors
    ~$ grep -Pnor --color=never --include='*.[chi]*' 'sfd_base \+ \d+ *\+* *\d*' ./Utest | sort -t: -k3.12n

It is also strongly recommended to use unique port numbers for testing to avoid
getting a delay to re-use an ip address. I start with test port number 50000
-and use a simple search that does not necessarily find only used port nummbers.
-But it doesn't matter as long as the new used number isn't found. I use this:

    ~$ # Get used port numbers
    ~$ grep -Phor --include='*.[chi]*' '5\d\d\d\d' ./Utest | sort -n | uniq

<br /><pre>
// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  &#60;Ingo&#64;Hoeft-online.de&#62;
// Redistribution only with this Copyright remark. Last modified: 2024-04-16
</pre>
