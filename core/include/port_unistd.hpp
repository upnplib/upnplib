// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-10-17

// Header file for portable <unistd.h>
// ===================================
// On MS Windows <unistd.h> isn't availabe. We can use <io.h> instead for most
// functions but it's not 100% compatible.

#ifndef INCLUDE_PORT_UNISTD_HPP
#define INCLUDE_PORT_UNISTD_HPP

#if _WIN32
#include <fcntl.h>
#include <io.h>
#define ssize_t int
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#else // WIN32
#include <unistd.h>
#endif // WIN32

#endif // INCLUDE_PORT_UNISTD_HPP
