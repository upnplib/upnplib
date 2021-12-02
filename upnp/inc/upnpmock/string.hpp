// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-12-02

#ifndef UPNP_STRINGIF_HPP
#define UPNP_STRINGIF_HPP

#include <string.h>

namespace upnp {

class Bstring {
    // Real class to call the system functions
  public:
    virtual ~Bstring() {}

    virtual char* strerror(int errnum) { return ::strerror(errnum); }
};

// Global pointer to the current object (real or mocked), will be modified by
// the constructor of the mock object.
extern Bstring* string_h;

// In the production code you just prefix the old system call with
// 'upnp::string_h->' so the new call looks like this:
//  upnp::string_h->strerror(0)

// For completeness (not used here): you can also create the object on the heap
// Istring* stringObj = new Bstring(); // need to address constructor with ()
// char* msg = stringObj->strerror(errno);
// delete stringObj; // Important to avoid memory leaks!

/* clang-format off
 * The following class should be copied to the test source. You do not need to
 * copy all MOCK_METHOD, only that are acually used. It is not a good
 * idea to move it here to the header. It uses googletest macros and you always
 * have to compile the code with googletest even for production and not used.

class Mock_string : public Bstring {
    // Class to mock the free system functions.
    Bstring* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_string() { m_oldptr = string_h; string_h = this; }
    virtual ~Mock_string() { string_h = m_oldptr; }

    MOCK_METHOD(char*, strerror, (int errnum), (override));
};

 * In a gtest you will instantiate the Mock class, prefered as protected member
 * variable at the constructor of the testsuite:

    Mock_string m_mocked_string;

 *  and call it with: m_mocked_string.strerror(..)
 * clang-format on
*/

} // namespace upnp

#endif // UPNP_STRINGIF_HPP
