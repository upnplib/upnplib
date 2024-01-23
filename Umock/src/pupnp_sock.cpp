// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-01-29

#include "umock/pupnp_sock.inc"

namespace umock {

// This constructor is used to inject the pointer to the real function.
PupnpSock::PupnpSock(PupnpSockReal* a_ptr_realObj) {
    m_ptr_workerObj = (PupnpSockInterface*)a_ptr_realObj;
}

// This constructor is used to inject the pointer to the mocking function.
PupnpSock::PupnpSock(PupnpSockInterface* a_ptr_mockObj) {
    m_ptr_oldObj = m_ptr_workerObj;
    m_ptr_workerObj = a_ptr_mockObj;
}

// The destructor is used to restore the old pointer.
PupnpSock::~PupnpSock() { m_ptr_workerObj = m_ptr_oldObj; }

// Methods
int PupnpSock::sock_make_blocking(SOCKET sock) {
    return m_ptr_workerObj->sock_make_blocking(sock);
}

int PupnpSock::sock_make_no_blocking(SOCKET sock) {
    return m_ptr_workerObj->sock_make_no_blocking(sock);
}

// On program start create an object and inject pointer to the real functions.
// This will exist until program end.
PupnpSockReal pupnp_sock_realObj;
PupnpSock pupnp_sock(&pupnp_sock_realObj);

} // namespace umock
