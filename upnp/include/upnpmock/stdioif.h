// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-10-06

#ifndef UPNP_STDIOIF_H
#define UPNP_STDIOIF_H

#include <stdio.h>

class Istdio {
    // Interface to stdio system calls
  public:
    virtual ~Istdio() {}
    virtual FILE* fopen(const char* pathname, const char* mode) = 0;
    virtual int fclose(FILE* stream) = 0;
};

Istdio* stdioif;

class Cstdio : public Istdio {
    // Real class to call the system functions.
  public:
    virtual ~Cstdio() {}

    // With the constructor initialize the pointer to the interface that may be
    // overwritten to point to a mock object instead.
    Cstdio() { stdioif = this; }

    FILE* fopen(const char* pathname, const char* mode) override {
        return ::fopen(pathname, mode);
    }

    int fclose(FILE* stream) override { return ::fclose(stream); }
};

// clang-format off
// This is the instance to call the system functions. This object is called
// with its pointer stdioif (see above) that is initialzed with the
// constructor. That pointer can be overwritten to point to a mock object
// instead.
Cstdio stdioObj;

// In the production code you must call it with, e.g.:
// stdioif->fopen(...)

/*
 * The following class should be coppied to the test source.

class Mock_stdio : public Istdio {
// Class to mock the free system functions.
  public:
    virtual ~Mock_stdio() {}
    Mock_stdio() { stdioif = this; }
    MOCK_METHOD(FILE*, fopen, (const char* pathname, const char* mode), (override));
    MOCK_METHOD(int, fclose, (FILE* stream), (override));
};

 * In a gtest you will instantiate the Mock class, prefered as protected member
 * variable:
 * class now, e.g.:

    Mock_stdio mocked_stdio;

 *  and call it with: mocked_stdio.fopen(...) (prefered)
 *  or                    stdioif->fopen(...)
 * clang-format on
*/

#endif // UPNP_STDIOIF_H
