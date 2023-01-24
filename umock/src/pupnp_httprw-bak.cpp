// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-11-25

#include "umock/pupnp_httprw.hpp"

int Check_Connect_And_Wait_Connection(SOCKET sock, int connect_res);
int private_connect(SOCKET sockfd, const struct sockaddr* serv_addr,
                    socklen_t addrlen);

namespace umock {

#ifndef UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS
int PupnpHttpRwReal::Check_Connect_And_Wait_Connection(SOCKET sock,
                                                       int connect_res) {
    return ::Check_Connect_And_Wait_Connection(sock, connect_res);
}
#endif
int PupnpHttpRwReal::private_connect(SOCKET sockfd,
                                     const struct sockaddr* serv_addr,
                                     socklen_t addrlen) {
    return ::private_connect(sockfd, serv_addr, addrlen);
}

// This constructor is used to inject the pointer to the real function.
PupnpHttpRw::PupnpHttpRw(PupnpHttpRwReal* a_ptr_realObj) {
    m_ptr_workerObj = (PupnpHttpRwInterface*)a_ptr_realObj;
}

// This constructor is used to inject the pointer to the mocking function.
PupnpHttpRw::PupnpHttpRw(PupnpHttpRwInterface* a_ptr_mockObj) {
    m_ptr_oldObj = m_ptr_workerObj;
    m_ptr_workerObj = a_ptr_mockObj;
}

// The destructor is ussed to restore the old pointer.
PupnpHttpRw::~PupnpHttpRw() { m_ptr_workerObj = m_ptr_oldObj; }

// Methods
#ifndef UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS
int PupnpHttpRw::Check_Connect_And_Wait_Connection(SOCKET sock,
                                                   int connect_res) {
    return m_ptr_workerObj->Check_Connect_And_Wait_Connection(sock,
                                                              connect_res);
}
#endif
int PupnpHttpRw::private_connect(SOCKET sockfd,
                                 const struct sockaddr* serv_addr,
                                 socklen_t addrlen) {
    return m_ptr_workerObj->private_connect(sockfd, serv_addr, addrlen);
}

// On program start create an object and inject pointer to the real functions.
// This will exist until program end.
PupnpHttpRwReal pupnp_httprw_realObj;
PupnpHttpRw pupnp_httprw(&pupnp_httprw_realObj);

} // namespace umock
