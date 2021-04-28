## Compile and run tests

To configure the gtest build you should do this in the upnplib/upnp/gtest
directory:

    rm -r ./build/ && cmake -B ./build/ .

To be able to compile a test you have to add an entry in the local
`./CMakeLists.txt`. Then you can compile the tests in the
local gtest directory with:

    cmake --build ./build

Then you can run single tests, for example with:

    ./build/test_template.a

If the output is messed up with logging information from the library,
redirect stderr to /dev/null:

    ./build/test_upnpapi.a 2>/dev/null

To run all tests just use a for loop:

    for f in ./build/*.a; do $f --gtest_color=yes; echo; done 2>&1 | less

## Skip tests on Github Workflow Actions

It is possible that you have tests which are failing because fixing the bug it
is showing takes some time. To be able to push bug fixes and create pull
requests you can skip tests if they run on the Github Workflow as Action. The
test checks if the environment variable `GITHUB_ACTIONS` exists and will then
skip the test. An example for this conditional check you can find in the
test_template.cpp. If you want to see what tests are skipped on Github Actions
you can execute the test for example with:

    GITHUB_ACTIONS="true" ./build/test_template.a

<br />
Author: 2021 - Ingo Höft, <Ingo@Hoeft-online.de>
last modified: 2021-04-24
