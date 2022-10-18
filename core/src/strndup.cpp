// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-10-20

#include <cstring>
#include <cstdlib> // for malloc

/* strndup() is a GNU extension. */
#if !HAVE_STRNDUP || defined(_WIN32)
char* strndup(const char* __string, size_t __n) {
    size_t strsize = strnlen(__string, __n);
    char* newstr = (char*)malloc(strsize + 1);
    if (newstr == NULL)
        return NULL;

    strncpy(newstr, __string, strsize);
    newstr[strsize] = 0;

    return newstr;
}
#endif /* HAVE_STRNDUP && !defined(_WIN32) */
