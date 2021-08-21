// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-08-20

#pragma once
#include "UpnpInet.h"
#include <ifaddrs.h>
#include <string>

const char* UpnpGetErrorMessage(int rc);

class CIfaddr4
// Tool to manage and fill a socket address structure. This is needed
// for mocked network interfaces.
{
    struct ifaddrs ifaddr;

    struct sockaddr_in ifa_addr;    // network address
    struct sockaddr_in ifa_netmask; // netmask
    struct sockaddr_in ifa_ifu; // broadcast addr or point-to-point dest addr

    std::string mIfname;    // interface name
    std::string mIfaddress; // interface ip address

    // clang-format off
    // the bitmask is the offset in the netmasks array.
    std::string netmasks[33] = {"0.0.0.0",
            "128.0.0.0", "192.0.0.0", "224.0.0.0", "240.0.0.0",
            "248.0.0.0", "252.0.0.0", "254.0.0.0", "255.0.0.0",
            "255.128.0.0", "255.192.0.0", "255.224.0.0", "255.240.0.0",
            "255.248.0.0", "255.252.0.0", "255.254.0.0", "255.255.0.0",
            "255.255.128.0", "255.255.192.0", "255.255.224.0", "255.255.240.0",
            "255.255.248.0", "255.255.252.0", "255.255.254.0", "255.255.255.0",
            "255.255.255.128", "255.255.255.192", "255.255.255.224", "255.255.255.240",
            "255.255.255.248", "255.255.255.252", "255.255.255.254", "255.255.255.255"};
    // clang-format on

  public:
    CIfaddr4();
    ifaddrs* get();
    bool set(std::string pIfname, std::string pIfaddress);
    void chain_next_addr(struct ifaddrs* ptrNextAddr);
};

class CIfaddr4Container;

class CCaptureFd
// Tool to capture output to a file descriptor, mainly used to capture program
// output to stdout or stderr.
// When printing the captured output, all opened file descriptor will be closed
// to avoid confusing output loops. For a new capture after print(..) you have
// to call capture(..) again.
{
    int fd;
    int fd_old;
    int fd_log;
    bool err = true;
    std::string captFname;

  public:
    CCaptureFd();
    ~CCaptureFd();
    void capture(int prmFd);
    bool print(std::ostream& pOut);

  private:
    void closeFds();
};
