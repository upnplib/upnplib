## pUPnP include files also used by the compa library
There are several include files that are also used by the compa library as long as they are not rewritten to a compatible version. It is needed to have theese files separated into a directory owned by the compa library to avoid conflicts with same file names on pUPnP and compa. To just build with the old native pUPnP library or with the compa library with same behavior, we must use same include filenames, but ensure to select the right include files with different search paths.

With theese commands we create first symlinks to the old include files and then overwrite them with the filenames that are different.

    find ../../../pupnp/upnp/inc/ -type f -name "*.hpp" -printf "ln -s %p %P\n" | sh
    find ../../../pupnp/upnp/src/inc/ -type f -name "*.hpp" -printf "ln -s %p %P\n" | sh
    find ../../../pupnp/upnp/src/threadutil/ -type f -name "*.hpp" -printf "ln -s %p %P\n" | sh
    rm miniserver.hpp
    ln -s compa/miniserver.hpp ../miniserver.hpp

<pre><sup>
// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, &#60;Ingo&#64;Hoeft-online.de&#62;
// Redistribution only with this Copyright remark. Last modified: 2023-07-13
</sup></sup>
