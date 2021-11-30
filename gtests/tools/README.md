Author: 2021-03-17 - Ingo Höft, last modified: 2021-03-17

Additional helper programs to initialize and improve tests.
You can compile it with ./compile.sh test_tools.cpp.

Example to compile tool tests on MS Windows with powershell:
PS: cd gtests\build/MinSizeRel
PS: cl /nologo /EHsc /std:c++17 /I..\..\../build\_deps\googletest-src\googletest\include ..\..\..\build\lib\MinSizeRel\gtest.lib ..\..\tools\test_container.cpp ws2_32.lib iphlpapi.lib

<pre><sup>
// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  &#60;Ingo&#64;Hoeft-online.de&#62;
// Redistribution only with this Copyright remark. Last modified: 2021-11-30
</sup></sup>
