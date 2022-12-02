## Symbol import issue with global variables on Microsoft Windows
Everything works as expected on Unix platforms but on MS Windows linking always fails due to missing symbols. Here I use `Stdlib stdlib_h' as example. I got the error message:

    UpnpString.obj : error LNK2001: unresolved external symbol "class upnplib::mocking::Stdlib upnplib::mocking::stdlib_h" (?stdlib_h@mocking@upnplib@@3VStdlib@12@A) [C:\Users\ingo\devel\upnplib\build\pupnp\upnp\pupnp_native_shared.vcxproj]

It took me a long time and much effort to understand what's going on here. Verifing with e.g.:

    dumpbin.exe /EXPORTS .\build\lib\Release\upnplib_native_shared.lib

I found that all needed symbols are exported and should be available to the linker. Finally I found the crucial hint on [CMake - WINDOWS_EXPORT_ALL_SYMBOLS](https://cmake.org/cmake/help/v3.18/prop_tgt/WINDOWS_EXPORT_ALL_SYMBOLS.html):

> For global data symbols, `__declspec(dllimport)` must still be used when compiling against the code in the .dll.

It was not a problem with export symbols but with import symbols of only global variables. I have to decorate the variable definiton with `__declspec(dllexport)` and its external usage declaration with `__declspec(dllimport) extern`. This cannot be done with one header file as done with the function symbols. There are some possibilities to achive this problem but I use this one with an additional include file (e.g. stdlib.inc) that contains only the function declariations so I can include this into the definition file (e.g. stdlib.cpp). The export decoration in stdlib.cpp is direct prepended to the global variable definition. The original header file (e.g. stdlib.hpp) contains also the import decoration to be used as external in other files. This solution has the advantage that it has no impact on the includes of existing code and that I can manage it here on one place. Please note that I use for the decorations the macros UPNPLIB_API and an additional created macro UPNPLIB_EXTERN to also match other build conditions and platforms. --Ingo

<pre><sup>
// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  &#60;Ingo&#64;Hoeft-online.de&#62;
// Redistribution only with this Copyright remark. Last modified: 2022-10-15
</sup></sup>
