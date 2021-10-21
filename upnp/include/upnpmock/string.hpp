// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-10-21

#ifndef UPNP_STRINGIF_H
#define UPNP_STRINGIF_H

#include <string.h>

namespace upnp {

class Istring {
    // Interface to system calls
  public:
    virtual ~Istring() {}
    virtual char* strerror(int errnum) = 0;
};

// Global pointer to the current object (real or mocked), will be set by the
// constructor of the respective object.
Istring* string_h;

class Cstring : public Istring {
    // Real class to call the system functions
  public:
    virtual ~Cstring() {}

    // With the constructor initialize the pointer to the interface that may be
    // overwritten to point to a mock object instead.
    Cstring() { string_h = this; }

    char* strerror(int errnum) override { return ::strerror(errnum); }
};

// clang-format off
// This is the instance to call the system functions. This object is called
// with its pointer string_h (see above) that is initialzed with the
// constructor. That pointer can be overwritten to point to a mock object
// instead.
Cstring stringObj;

// In the production code you must call it with, e.g.:
// upnp::string_h->strerror(0)

// For completeness (not used here): you can also create the object on the heap
// Istring* stringObj = new Cstring(); // need to address constructor with ()
// char* msg = stringObj->strerror(errno);
// delete stringObj; // Important to avoid memory leaks!

/*
 * The following class should be coppied to the test source. It is not a good
 * idea to move it here to the header. It uses googletest macros and you always
 * have to compile the code with googletest even for production and not used.

class Mock_string : public Istring {
// Class to mock the free system functions.
    Istring* m_oldptr;
  public:
    // Save and restore the old pointer to the production function
    Mock_string() { m_oldptr = string_h; string_h = this; }
    virtual ~Mock_string() { string_h = m_oldptr; }

    MOCK_METHOD(char*, strerror, (int errnum), (override));
};

 * In a gtest you will instantiate the Mock class, prefered as protected member
 * variable for the whole testsuite:

    Mock_string mocked_string;

 *  and call it with: mocked_string.strerror(..)
 * clang-format on
*/

} // namespace upnp

#endif // UPNP_STRINGIF_H
