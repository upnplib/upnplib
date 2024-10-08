#ifndef PUPNP_POSIX_OVERWRITES_HPP
#define PUPNP_POSIX_OVERWRITES_HPP
#ifdef _WIN32
// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-10-07
// Last compare with pupnp original source file on 2024-08-01, ver 1.14.19

/* POSIX names for functions */
#define fileno _fileno
#define unlink _unlink
#define strcasecmp _stricmp
// #define strdup _strdup
#define stricmp _stricmp
#define strncasecmp strnicmp
#define strnicmp _strnicmp

/* Secure versions of functions */
/* Explicitly disable warnings by pragma/define, see:
 * https://www.codegrepper.com/code-examples/c/crt+secure+no+warnings */
#pragma warning(disable : 4996)
#define _CRT_SECURE_NO_WARNINGS
#if 0
        /*
         * The current issues with those 4 defines:
         * - strncpy redefinition is wrong
         * - Theses functions assume they are being called on C arrays
         * only. Using `countof` on a heap allocated pointer is
         * undefined behavior and `sizeof` will only return the byte
         * size of the pointer.
         *
         * The reason we can't pin-point the places where it fails is
         * because *_s functions have a significantly different
         * behaviour than the replaced functions and have actual error
         * returns values that are simply ignored here, leading to
         * numerous unseen regressions.
         *
         * A first step could be to actually crash or log on _s failures
         * to detect the potentials overflows or bad usages of the
         * wrappers.
         */
#define strcat(arg1, arg2) strcat_s(arg1, sizeof(arg1), arg2)
#define strcpy(arg1, arg2) strcpy_s(arg1, _countof(arg1), arg2)
#define strncpy(arg1, arg2, arg3) strncpy_s(arg1, arg3, arg2, arg3)
#define sprintf(arg1, ...) sprintf_s(arg1, sizeof(arg1), __VA_ARGS__)
#endif

#endif /* _WIN32 */
#endif /* PUPNP_POSIX_OVERWRITES_HPP */
