// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-03-10

#include <iostream>

#if defined(COMPA_HAVE_MINISERVER) && defined(UPNP_ENABLE_OPEN_SSL)
#include <upnp.hpp>

int sample001() {
    int ret = UpnpInitSslContext(1, TLS_method());
    if (ret != UPNP_E_SUCCESS)
        return 1;

    ret = UpnpFinish();
    if (ret != UPNP_E_SUCCESS)
        return 2;

    return 0;
}
#endif


int main() {
    int retcode{};

#if defined(COMPA_HAVE_MINISERVER) && defined(UPNP_ENABLE_OPEN_SSL)
    retcode = sample001();
#endif

    if (retcode)
        std::cerr << "Exit with return code " << retcode << std::endl;
    return retcode;
}
