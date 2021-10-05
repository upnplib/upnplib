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

class Cstdio : public Istdio {
    // Real class to call the system functions.
  public:
    virtual ~Cstdio() {}

    FILE* fopen(const char* pathname, const char* mode) override {
        return ::fopen(pathname, mode);
    }

    int fclose(FILE* stream) override { return ::fclose(stream); }
};

// clang-format off
// This is the instance to call the system functions.
// It is initialized by default.
Cstdio stdioObj;
Istdio* stdio = &stdioObj;

// In the production code you must call it with, e.g.:
// pthread->pthread_mutex_init(...)

/*
 * The following class should be coppied to the test source.

class Mock_stdio : public Istdio {
// Class to mock the free system functions.
  public:
    virtual ~Mock_stdio() {}
    MOCK_METHOD(FILE*, fopen, (const char* pathname, const char* mode), (override));
    MOCK_METHOD(int, fclose, (FILE* stream), (override));
};

 * In a test macro you will instantiate the Mock class and overwrite the pointer
 * "Istdio* stdio" (look above) to the interface so it points to the mocking
 * class now, e.g.:

    Mock_stdio mocked_stdio;
    stdio = &mocked_stdio;

 *  and call it with: mocked_stdio.fopen(...) (prefered)
 *  or                      stdio->fopen(...)
 * clang-format on
*/

#endif // UPNP_STDIOIF_H
