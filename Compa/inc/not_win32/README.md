This directory contains empty header files. This will be included by the linker when it cannot find header files from other platforms. For example on MS Windows we need <winsock2.h> but not on Unix platforms. To manage this we can use preprocessor macros:

    #ifdef _WIN32
    #include <winsock2.h>
    #endif

This must be done on every source file. To avoid this we can just use

    #include "winsock2.h"

If we are on MS Windows the linker will not find a "winsock2.h" file and falls back to use the correct <winsock2.h>. This is managed by CMake with:

    include_directories(
        $<$<NOT:$<BOOL:${WIN32}>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
    )

If not on MS Windows the linker finds there an empty winsock2.h file that just do nothing as before but don't emit an error that the include file cannot be found.
