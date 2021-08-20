// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-08-20

// Tools and helper classes to manage gtests
// =========================================

#include "tools.h"
#include "upnp.h"
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <vector>

// Errormessages taken from https://github.com/pupnp/pupnp
// Author: Marcelo Roberto Jimenez <marcelo.jimenez@gmail.com>
//------------------------------------------------------------
/*!
 * \brief Structure to maintain a error code and string associated with the
 * error code.
 */
struct ErrorString {
    /*! Error code. */
    int rc;
    /*! Error description. */
    const char* rcError;
};

/*!
 * \brief Array of error structures.
 */
struct ErrorString ErrorMessages[] = {
    {UPNP_E_SUCCESS, "UPNP_E_SUCCESS"},
    {UPNP_E_INVALID_HANDLE, "UPNP_E_INVALID_HANDLE"},
    {UPNP_E_INVALID_PARAM, "UPNP_E_INVALID_PARAM"},
    {UPNP_E_OUTOF_HANDLE, "UPNP_E_OUTOF_HANDLE"},
    {UPNP_E_OUTOF_CONTEXT, "UPNP_E_OUTOF_CONTEXT"},
    {UPNP_E_OUTOF_MEMORY, "UPNP_E_OUTOF_MEMORY"},
    {UPNP_E_INIT, "UPNP_E_INIT"},
    {UPNP_E_BUFFER_TOO_SMALL, "UPNP_E_BUFFER_TOO_SMALL"},
    {UPNP_E_INVALID_DESC, "UPNP_E_INVALID_DESC"},
    {UPNP_E_INVALID_URL, "UPNP_E_INVALID_URL"},
    {UPNP_E_INVALID_SID, "UPNP_E_INVALID_SID"},
    {UPNP_E_INVALID_DEVICE, "UPNP_E_INVALID_DEVICE"},
    {UPNP_E_INVALID_SERVICE, "UPNP_E_INVALID_SERVICE"},
    {UPNP_E_BAD_RESPONSE, "UPNP_E_BAD_RESPONSE"},
    {UPNP_E_BAD_REQUEST, "UPNP_E_BAD_REQUEST"},
    {UPNP_E_INVALID_ACTION, "UPNP_E_INVALID_ACTION"},
    {UPNP_E_FINISH, "UPNP_E_FINISH"},
    {UPNP_E_INIT_FAILED, "UPNP_E_INIT_FAILED"},
    {UPNP_E_URL_TOO_BIG, "UPNP_E_URL_TOO_BIG"},
    {UPNP_E_BAD_HTTPMSG, "UPNP_E_BAD_HTTPMSG"},
    {UPNP_E_ALREADY_REGISTERED, "UPNP_E_ALREADY_REGISTERED"},
    {UPNP_E_INVALID_INTERFACE, "UPNP_E_INVALID_INTERFACE"},
    {UPNP_E_NETWORK_ERROR, "UPNP_E_NETWORK_ERROR"},
    {UPNP_E_SOCKET_WRITE, "UPNP_E_SOCKET_WRITE"},
    {UPNP_E_SOCKET_READ, "UPNP_E_SOCKET_READ"},
    {UPNP_E_SOCKET_BIND, "UPNP_E_SOCKET_BIND"},
    {UPNP_E_SOCKET_CONNECT, "UPNP_E_SOCKET_CONNECT"},
    {UPNP_E_OUTOF_SOCKET, "UPNP_E_OUTOF_SOCKET"},
    {UPNP_E_LISTEN, "UPNP_E_LISTEN"},
    {UPNP_E_TIMEDOUT, "UPNP_E_TIMEDOUT"},
    {UPNP_E_SOCKET_ERROR, "UPNP_E_SOCKET_ERROR"},
    {UPNP_E_FILE_WRITE_ERROR, "UPNP_E_FILE_WRITE_ERROR"},
    {UPNP_E_CANCELED, "UPNP_E_CANCELED"},
    {UPNP_E_EVENT_PROTOCOL, "UPNP_E_EVENT_PROTOCOL"},
    {UPNP_E_SUBSCRIBE_UNACCEPTED, "UPNP_E_SUBSCRIBE_UNACCEPTED"},
    {UPNP_E_UNSUBSCRIBE_UNACCEPTED, "UPNP_E_UNSUBSCRIBE_UNACCEPTED"},
    {UPNP_E_NOTIFY_UNACCEPTED, "UPNP_E_NOTIFY_UNACCEPTED"},
    {UPNP_E_INVALID_ARGUMENT, "UPNP_E_INVALID_ARGUMENT"},
    {UPNP_E_FILE_NOT_FOUND, "UPNP_E_FILE_NOT_FOUND"},
    {UPNP_E_FILE_READ_ERROR, "UPNP_E_FILE_READ_ERROR"},
    {UPNP_E_EXT_NOT_XML, "UPNP_E_EXT_NOT_XML"},
    {UPNP_E_NO_WEB_SERVER, "UPNP_E_NO_WEB_SERVER"},
    {UPNP_E_OUTOF_BOUNDS, "UPNP_E_OUTOF_BOUNDS"},
    {UPNP_E_NOT_FOUND, "UPNP_E_NOT_FOUND"},
    {UPNP_E_INTERNAL_ERROR, "UPNP_E_INTERNAL_ERROR"},
};

const char* UpnpGetErrorMessage(int rc) {
    size_t i;

    for (i = 0; i < sizeof(ErrorMessages) / sizeof(ErrorMessages[0]); ++i) {
        if (rc == ErrorMessages[i].rc) {
            return ErrorMessages[i].rcError;
        }
    }

    return "Unknown error code";
}

CIfaddr4::CIfaddr4()
// With constructing the object you get a loopback device by default.
{
    // loopback interface
    //-------------------
    // set network address
    ifa_addr.sin_family = AF_INET;
    // ifa_addr.sin_port = htons(MYPORT);
    inet_aton("127.0.0.1", &(ifa_addr.sin_addr));

    // set netmask
    ifa_netmask.sin_family = AF_INET;
    // ifa_netmask.sin_port = htons(MYPORT);
    inet_aton("255.0.0.0", &(ifa_netmask.sin_addr));

    // set broadcast address or Point-to-point destination address
    ifa_ifu.sin_family = AF_INET;
    // ifa_ifu.sin_port = htons(MYPORT);
    inet_aton("0.0.0.0", &(ifa_ifu.sin_addr));

    this->ifaddr.ifa_next = nullptr; // pointer to next ifaddrs structure
    this->ifaddr.ifa_name = (char*)"lo";
    // v-- Flags from SIOCGIFFLAGS, man 7 netdevice
    this->ifaddr.ifa_flags = 0 | IFF_LOOPBACK | IFF_UP;
    this->ifaddr.ifa_addr = (struct sockaddr*)&ifa_addr;
    this->ifaddr.ifa_netmask = (struct sockaddr*)&ifa_netmask;
    this->ifaddr.ifa_broadaddr = (struct sockaddr*)&ifa_ifu;
    this->ifaddr.ifa_data = nullptr;
}

ifaddrs* CIfaddr4::get()
// Return the pointer to the ifaddr structure
{
    return &this->ifaddr;
}

bool CIfaddr4::set(std::string pIfname, std::string pIfaddress)
// Set the interface name and the ipv4 address with bitmask. Properties are
// set to an ipv4 UP interface, supporting broadcast and multicast.
// Returns true if successful.
{
    if (pIfname == "" or pIfaddress == "")
        return false;

    // to be thread save we will have the strings here
    this->mIfname = pIfname;
    this->mIfaddress = pIfaddress;
    this->ifaddr.ifa_name = (char*)this->mIfname.c_str();

    // get the netmask from the bitmask
    // the bitmask is the offset in the netmasks array.
    std::size_t slashpos = this->mIfaddress.find_first_of("/");
    std::string address = this->mIfaddress;
    std::string bitmask = "32";
    if (slashpos != std::string::npos) {
        address = this->mIfaddress.substr(0, slashpos);
        bitmask = this->mIfaddress.substr(slashpos + 1);
    }
    // std::cout << "DEBUG! set ifa_name: " << this->ifaddr.ifa_name << ",
    // address: '" << address << "', bitmask: '" << bitmask << "', netmask: " <<
    // netmasks[std::stoi(bitmask)] << ", slashpos: " << slashpos << "\n";

    // convert address strings to numbers and store them
    inet_aton(address.c_str(), &(this->ifa_addr.sin_addr));
    std::string netmask = netmasks[std::stoi(bitmask)];
    inet_aton(netmask.c_str(), &(this->ifa_netmask.sin_addr));
    this->ifaddr.ifa_flags = 0 | IFF_UP | IFF_BROADCAST | IFF_MULTICAST;

    // calculate broadcast address as follows: broadcast = ip | ( ~ subnet )
    // broadcast = ip-addr ored the inverted subnet-mask
    this->ifa_ifu.sin_addr.s_addr =
        this->ifa_addr.sin_addr.s_addr | ~this->ifa_netmask.sin_addr.s_addr;

    this->ifaddr.ifa_addr = (struct sockaddr*)&this->ifa_addr;
    this->ifaddr.ifa_netmask = (struct sockaddr*)&this->ifa_netmask;
    this->ifaddr.ifa_broadaddr = (struct sockaddr*)&this->ifa_ifu;
    return true;
}

void CIfaddr4::chain_next_addr(struct ifaddrs* ptrNextAddr) {
    this->ifaddr.ifa_next = ptrNextAddr;
}

class CIfaddr4Container
// This is a Container for multiple network interface structures that are
// chained by ifaddr.ifa_next as given by the low level struct ifaddrs.
//
// It is IMPORTANT to know that the ifaddr.ifa_next address pointer chain
// changes when adding an interface address object. You MUST get_ifaddr(..)
// the new address pointer for ongoing work.
{
    std::vector<CIfaddr4> ifaddr4Container;

  public:
    bool add(std::string prmIfname, std::string prmIfaddress) {
        ifaddrs* ifaddr4New;

        if (prmIfname == "lo" and prmIfaddress != "127.0.0.1/8")
            return false;

        if (this->ifaddr4Container.empty()) {
            // On an empty container just push an interface object to it
            this->ifaddr4Container.push_back(CIfaddr4());
            if (prmIfname == "lo")
                return true;
            return this->ifaddr4Container[0].set(prmIfname, prmIfaddress);
        }

        // If the container is not empty, get the position to the last
        // interface object, append a new interface object and chain it to the
        // low level structure ifaddrs, managed by previous interface object.
        // pos is zero based, so the size points direct to a new entry.
        int pos = this->ifaddr4Container.size();
        this->ifaddr4Container.push_back(CIfaddr4());

        // Because the ifaddr.ifa_next address pointer chain changes when adding
        // an interface object, the chain must be rebuild
        for (int i = 1; i <= pos; i++) {
            ifaddr4New = this->ifaddr4Container.at(i).get();
            this->ifaddr4Container.at(i - 1).chain_next_addr(ifaddr4New);
        }

        if (prmIfname == "lo")
            return true;
        return this->ifaddr4Container.at(pos).set(prmIfname, prmIfaddress);
    }

    ifaddrs* get_ifaddr(long unsigned int pIdx) {
        if (pIdx == 0)
            return nullptr;

        // this throws an exception if vector.size() is violated
        ifaddrs* ifaddr4 = this->ifaddr4Container.at(pIdx - 1).get();
        return ifaddr4;
    }
};

CCaptureFd::CCaptureFd() {
    // generate random temporary filename to be thread-safe
    std::srand(std::time(nullptr));
    this->captFname = (std::string)std::filesystem::temp_directory_path() +
                      "/gtestcapt" + std::to_string(std::rand());
}

CCaptureFd::~CCaptureFd() {
    this->closeFds();
    remove(this->captFname.c_str());
}

void CCaptureFd::capture(int prmFd) {
    this->fd = prmFd;
    this->fd_old = ::dup(prmFd);
    if (this->fd_old < 0) {
        return;
    }
    this->fd_log =
        ::open(this->captFname.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0660);
    if (this->fd_log < 0) {
        ::close(this->fd_old);
        return;
    }
    if (::dup2(this->fd_log, prmFd) < -2) {
        ::close(this->fd_old);
        ::close(this->fd_log);
        return;
    }
    this->err = false;
}

bool CCaptureFd::print(std::ostream& pOut)
// Close all file descriptors and print captured file content.
// If nothing was captured, then the return value is false.
{
    if (this->err)
        return false;
    this->closeFds();

    std::ifstream readFileObj(this->captFname.c_str());
    std::string lineBuf = "";

    std::getline(readFileObj, lineBuf);
    if (lineBuf == "") {
        readFileObj.close();
        remove(this->captFname.c_str());
        return false;
    }

    pOut << lineBuf << "\n";
    while (std::getline(readFileObj, lineBuf))
        pOut << lineBuf << "\n";

    readFileObj.close();
    remove(this->captFname.c_str());
    return true;
}

void CCaptureFd::closeFds() {
    // restore old fd
    ::dup2(this->fd_old, this->fd);
    ::close(this->fd_old);
    ::close(this->fd_log);
    this->err = true;
}
