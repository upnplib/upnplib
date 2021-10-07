// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-10-07

#ifndef UPNP_STRINGIF_H
#define UPNP_STRINGIF_H

#include <string.h>

class Istring {
    // Interface to system calls
  public:
    virtual ~Istring() {}
    virtual char* strerror(int errnum) = 0;
};

// Global pointer to the current object (real or mocked), will be set by the
// constructor of the respective object.
Istring* stringif;

class Cstring : public Istring {
    // Real class to call the system functions
  public:
    virtual ~Cstring() {}

    // With the constructor initialize the pointer to the interface that may be
    // overwritten to point to a mock object instead.
    Cstring() { stringif = this; }

    char* strerror(int errnum) override { return ::strerror(errnum); }
};

// clang-format off
// This is the instance to call the system functions. This object is called
// with its pointer stringif (see above) that is initialzed with the
// constructor. That pointer can be overwritten to point to a mock object
// instead.
Cstring stringObj;

// In the production code you must call it with, e.g.:
// stringif->strerror(0)

/*
 * The following class should be coppied to the test source. It is not a good
 * idea to move it here to the header. It uses googletest macros and you always
 * have to compile the code with googletest even for production and not used.

class Mock_string : public Istring {
// Class to mock the free system functions.
  public:
    virtual ~Mock_string() {}
    Mock_string() { stringif = this; }
    MOCK_METHOD(char*, strerror, (int errnum), (override));
};

 * In a gtest you will instantiate the Mock class, prefered as protected member
 * variable for the whole testsuite:

    Mock_string mocked_string;

 *  and call it with: mocked_string.strerror(..) (prefered)
 *  or                    stringif->strerror(..)
 * clang-format on
*/

#endif // UPNP_STRINGIF_H
