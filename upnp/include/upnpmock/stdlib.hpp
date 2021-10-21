// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-10-21

#ifndef UPNP_STDLIBIF_H
#define UPNP_STDLIBIF_H

#include <stdlib.h>

namespace upnp {

class Istdlib {
    // Interface to system calls
  public:
    virtual ~Istdlib() {}
    virtual void* malloc(size_t size) = 0;
    virtual void free(void* ptr) = 0;
};

// Global pointer to the current object (real or mocked), will be set by the
// constructor of the respective object.
Istdlib* stdlib_h;

class Cstdlib : public Istdlib {
    // Real class to call the system functions
  public:
    virtual ~Cstdlib() {}

    // With the constructor initialize the pointer to the interface that may be
    // overwritten to point to a mock object instead.
    Cstdlib() { stdlib_h = this; }

    void* malloc(size_t size) override { return ::malloc(size); }
    void free(void* ptr) override { return ::free(ptr); }
};

// clang-format off
// This is the instance to call the system functions. This object is called
// with its pointer stdlib_h (see above) that is initialzed with the
// constructor. That pointer can be overwritten to point to a mock object
// instead.
Cstdlib stdlibObj;

// In the production code you must call it with, e.g.:
// upnp::stdlib_h->malloc(sizeof(whatever))

/*
 * The following class should be coppied to the test source. It is not a good
 * idea to move it here to the header. It uses googletest macros and you always
 * have to compile the code with googletest even for production and not used.

class Mock_stdlib : public Istdlib {
// Class to mock the free system functions.
    Istdlib* m_oldptr;
  public:
    // Save and restore the old pointer to the production function
    Mock_stdlib() { m_oldptr = stdlib_h; stdlib_h = this; }
    virtual ~Mock_stdlib() { stdlib_h = m_oldptr; }

    MOCK_METHOD(void*, malloc, (size_t size), (override));
    MOCK_METHOD(void, free, (void* ptr), (override));
};

 * In a gtest you will instantiate the Mock class, prefered as protected member
 * variable for the whole testsuite:

    Mock_stdlib mocked_stdlib;

 *  and call it with: mocked_stdlib.malloc(sizeof(whatever));
 * clang-format on
*/

} // namespace upnp

#endif // UPNP_STDLIBIF_H
