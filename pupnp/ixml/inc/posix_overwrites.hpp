#ifndef PUPNP_POSIX_OVERWRTIES_HPP
#define PUPNP_POSIX_OVERWRTIES_HPP
#ifdef _WIN32
// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-09-08

/* POSIX names for functions */
#define fileno _fileno
#define unlink _unlink
#define strcasecmp _stricmp
#define strdup _strdup
#define stricmp _stricmp
#define strncasecmp strnicmp
#define strnicmp _strnicmp

/* Secure versions of functions */
// Ingo: Not enabled as described at
// https://github.com/upnplib/upnplib/issues/11.
// #define strcat(arg1, arg2) strcat_s(arg1, sizeof(arg1), arg2)
// #define strcpy(arg1, arg2) strcpy_s(arg1, _countof(arg1), arg2)
// #define strncpy(arg1, arg2, arg3) strncpy_s(arg1, arg3, arg2, arg3)
// #define sprintf(arg1, ...) sprintf_s(arg1, sizeof(arg1), __VA_ARGS__)

#endif /* _WIN32 */
#endif /* PUPNP_POSIX_OVERWRTIES_HPP */
