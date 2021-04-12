// Tools and helper classes to manage gtests
// Author: 2021-03-12 - Ingo Höft

#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <vector>
#include <variant>
#include <unistd.h>


class CIfaddr4
// Tool to manage and fill a socket address structure. This is needed
// for mocked network interfaces.
{
    // this is the ifaddr structure to filled and returned
    struct ifaddrs ifaddr;

    struct sockaddr_in ifa_addr;      // network address
    struct sockaddr_in ifa_netmask;   // netmask
    struct sockaddr_in ifa_ifu;    // broadcast addr or point-to-point dest addr

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
public:
    CIfaddr4()
    // With constructing the object you get a loopback device by default.
    {
        // loopback interface
        //-------------------
        // set network address
        ifa_addr.sin_family = AF_INET;
        //ifa_addr.sin_port = htons(MYPORT);
        inet_aton("127.0.0.1", &(ifa_addr.sin_addr));

        // set netmask
        ifa_netmask.sin_family = AF_INET;
        //ifa_netmask.sin_port = htons(MYPORT);
        inet_aton("255.0.0.0", &(ifa_netmask.sin_addr));

        // set broadcast address or Point-to-point destination address
        ifa_ifu.sin_family = AF_INET;
        //ifa_ifu.sin_port = htons(MYPORT);
        inet_aton("0.0.0.0", &(ifa_ifu.sin_addr));

        ifaddr.ifa_next = nullptr;  // pointer to next ifaddrs structure
        ifaddr.ifa_name = (char*)"lo";
        // v-- Flags from SIOCGIFFLAGS, man 7 netdevice
        ifaddr.ifa_flags = 0 | IFF_LOOPBACK | IFF_UP;
        ifaddr.ifa_addr = (struct sockaddr*)&ifa_addr;
        ifaddr.ifa_netmask = (struct sockaddr*)&ifa_netmask;
        ifaddr.ifa_broadaddr = (struct sockaddr*)&ifa_ifu;
        ifaddr.ifa_data = nullptr;
    }


    ifaddrs* get()
    // Return the pointer to the ifaddr structure
    {
        return &ifaddr;
    }


    bool set(std::string pIfname, std::string pIfaddress)
    // Set the interface name and the ipv4 address with bitmask. Properties are
    // set to an ipv4 UP interface, supporting broadcast and multicast.
    // Returns true if successful.
    {
        ifaddr.ifa_name = (char*)pIfname.c_str();

        // get the netmask from the bitmask
        // the bitmask is the offset in the netmasks array.
        std::size_t slashpos = pIfaddress.find_first_of("/");
        std::string address = pIfaddress;
        std::string bitmask = "32";
        if (slashpos != std::string::npos) {
            address = pIfaddress.substr(0,slashpos);
            bitmask = pIfaddress.substr(slashpos+1);
        }
        //std::cout << "address: '" << address << "', bitmask: '" << bitmask << "', netmask: "
        //          << netmasks[std::stoi(bitmask)] << ", slashpos: " << slashpos << "\n";

        // convert address strings to numbers and store them
        inet_aton(address.c_str(), &(ifa_addr.sin_addr));
        std::string netmask = netmasks[std::stoi(bitmask)];
        inet_aton(netmask.c_str(), &(ifa_netmask.sin_addr));
        ifaddr.ifa_flags = 0 | IFF_UP | IFF_BROADCAST | IFF_MULTICAST;

        // calculate broadcast address as follows: broadcast = ip | ( ~ subnet )
        // broadcast = ip-addr ored the inverted subnet-mask
        ifa_ifu.sin_addr.s_addr = ifa_addr.sin_addr.s_addr | ~ ifa_netmask.sin_addr.s_addr;

        return true;
    }


    void set_next_addr(struct ifaddrs* ptrNextAddr)
    {
        ifaddr.ifa_next = ptrNextAddr;
    }
};


class CIfaddr4Container
// This is a Container for multiple network interface structures that are
// chained by ifaddr.ifa_next as given by the low level struct ifaddrs.
{
    // Have a container with two address types
    std::vector<CIfaddr4> ifaddr4Container;

public:
    bool add(std::string pIfname, std::string pIfaddress)
    {
        CIfaddr4 ifaddr4Obj;

        if (this->ifaddr4Container.empty())
        {
            if ( ! ifaddr4Obj.set(pIfname, pIfaddress))
                return false;

            // On an empty container just push an interface object to it
            //std::cout << this << ": " << ifaddr4Obj.get()->ifa_name << "\n";
            ifaddr4Obj.get();
            //ifaddr4Obj.get()->ifa_name = (char*)"yyyyy";
            this->ifaddr4Container.push_back(ifaddr4Obj);
            this->ifaddr4Container[0].get();
            //CIfaddr4* ifaddr4O = this->ifaddr4Container.data();
            //std::cout << this << ": " << ifaddr4O->get()->ifa_name << "\n";
            //this->ifaddr4Container[0].get()->ifa_name = (char*)"xxxxx";
            //std::cout << "here " << this->ifaddr4Container[0].get()->ifa_name << "\n";
            return true;
        }
        return false;

        // If the container is not empty, get the pointer to the last
        // interface object, append a new interface object and chain it to the
        // low level structure ifaddrs, managed by previous interface object.
        CIfaddr4 ifaddr4Prev = this->ifaddr4Container.back();
        this->ifaddr4Container.push_back (CIfaddr4());
        CIfaddr4 ifaddr4New = this->ifaddr4Container.back();
        ifaddr4Prev.set_next_addr(ifaddr4New.get());
        return ifaddr4New.set(pIfname, pIfaddress);
    }

    ifaddrs* ifaddr(long unsigned int pIdx)
    {
        std::cout << pIdx << "\n";
        if (pIdx < 1 or pIdx > this->ifaddr4Container.size())
            return nullptr;

        std::cout << this << " " << this->ifaddr4Container[0].get()->ifa_name << "\n";
        CIfaddr4 ifaddr4Obj = this->ifaddr4Container[0];
        ifaddrs* ifad = ifaddr4Obj.get();
        //std::cout << this << ": " << ifad->ifa_name << "\n";
        return ifad;
    }
};


class CTestClass
{
    int mVal;
public:
    CTestClass() {
        this->mVal = 123;
    }
    int get() {
        return this->mVal;
    }
    void set(int pVal) {
        this->mVal = pVal;
        return;
    }
};


class CTestContainer
{
    std::vector<CTestClass>testContainer;
    //CTestClass testClassObj;

public:
    void add(int pVal)
    {
        this->testContainer.push_back(CTestClass());
        CTestClass testObj = testContainer.back();
        std::cout << "val is '" << testObj.get() << "'\n";
        return;
    }

    void get()
    {
        std::cout << "get: '" << testContainer[0].get() << "'\n";
    }
};


class CCaptureFd
// Tool to capture output to a file descriptor, mainly used to capture program
// output to stdout or stderr.
// When printing the captured output, all opened file descriptor will be closed
// to avoid confusing output loops. For a new capture after print(..) you have
// to call capture(..) again.
{
    int mFd;
    int mFd_old;
    int mFd_log;
    bool mErr = true;
    char mCaptFname[16] = ".captfd.log";

public:
    void capture(int fd)
    {
        mFd = fd;
        mFd_old = ::dup(fd);
        if (mFd_old < 0) {
            return;
        }
        mFd_log = ::open(mCaptFname, O_WRONLY|O_CREAT|O_TRUNC, 0660);
        if (mFd_log < 0) {
            ::close(mFd_old);
            return;
        }
        if (::dup2(mFd_log, fd) < 0) {
            ::close(mFd_old);
            ::close(mFd_log);
            return;
        }
        mErr = false;
    }

    ~CCaptureFd()
    {
        this->closeFds();
        remove(mCaptFname);
    }

    bool print(std::ostream &pOut)
    // Close all file descriptors and print captured file content.
    // If nothing was captured, then the return value is false.
    {
        if (mErr) return false;
        this->closeFds();

        std::ifstream readFileObj(mCaptFname);
        std::string lineBuf = "";

        std::getline(readFileObj, lineBuf);
        if (lineBuf == "") {
            readFileObj.close();
            remove(mCaptFname);
            return false;
        }

        pOut << lineBuf << "\n";
        while (std::getline(readFileObj, lineBuf))
                pOut << lineBuf << "\n";

        readFileObj.close();
        remove(mCaptFname);
        return true;
    }

private:
    void closeFds()
    {
        // restore old fd
        ::dup2(mFd_old, mFd);
        ::close(mFd_old);
        ::close(mFd_log);
        mErr = true;
    }
};
