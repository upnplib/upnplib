// Compile with:
// g++ -std=c++23 -Wall -Wpedantic -Wextra -Werror -oUpnpInit2 \
//     -I/usr/local/include UpnpInit2.cpp -lupnpsdk-pupnp
// Execute with:
// LD_LIBRARY_PATH=/usr/local/lib ./UpnpInit2

#include <upnp.h>
#include <iostream>

int main() {
    constexpr char INTERFACE[] = "ens2";
    unsigned short PORT = 51515;

    int rc = UpnpInit2(INTERFACE, PORT);

    std::cout << "UpnpInit2(interface=" << INTERFACE << ", port=" << PORT
              << "), return code=" << rc << ".\n";
    // printf("UpnpInit2(interface=%s, port=%d) return code: %s (%d)\n",
    //        INTERFACE, PORT, UpnpGetErrorMessage(rc), rc);

    UpnpFinish();
    return rc != 0;
}
