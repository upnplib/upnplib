// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-11-01

#ifndef UPNP_STDLIBIF_HPP
#define UPNP_STDLIBIF_HPP

#include <stdlib.h>

namespace upnp {

// Global pointer to the current object (real or mocked), will be set by the
// constructor of the respective object.
class Bstdlib; // Declaration of the class for the following pointer.
extern Bstdlib* stdlib_h;

class Bstdlib {
    // Real class to call the system functions
  public:
    virtual ~Bstdlib() {}

    // With the constructor initialize the pointer to the interface that may be
    // overwritten to point to a mock object instead.
    Bstdlib() { stdlib_h = this; }

    virtual void* malloc(size_t size) { return ::malloc(size); }
    virtual void free(void* ptr) { return ::free(ptr); }
};

// This is the instance to call the system functions. This object is called
// with its pointer stdlib_h (see above) that is initialzed with the
// constructor. That pointer can be overwritten to point to a mock object
// instead.
extern Bstdlib stdlibObj;

// In the production code you just prefix the old system call with
// 'upnp::stdlib_h->' so the new call looks like this:
//  upnp::stdlib_h->malloc(sizeof(whatever))

/* clang-format off
 * The following class should be copied to the test source. You do not need to
 * copy all MOCK_METHOD, only that are acually used. It is not a good
 * idea to move it here to the header. It uses googletest macros and you always
 * have to compile the code with googletest even for production and not used.

class Mock_stdlib : public Bstdlib {
    // Class to mock the free system functions.
    Bstdlib* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_stdlib() { m_oldptr = stdlib_h; stdlib_h = this; }
    virtual ~Mock_stdlib() { stdlib_h = m_oldptr; }

    MOCK_METHOD(void*, malloc, (size_t size), (override));
    MOCK_METHOD(void, free, (void* ptr), (override));
};

 * In a gtest you will instantiate the Mock class, prefered as protected member
 * variable at the constructor of the testsuite:

    Mock_stdlib m_mocked_stdlib;

 *  and call it with: m_mocked_stdlib.malloc(sizeof(whatever));
 * clang-format on
*/

} // namespace upnp

#endif // UPNP_STDLIBIF_HPP
