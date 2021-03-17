// Tool to manage and fill a socket address structure. This is needed
// for mocked network interfaces.
// Author: 2021-03-12 - Ingo Höft <Ingo@Hoeft-online.de>

#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>

class IfaddrInterface {
public:
    virtual ~IfaddrInterface() {}
    virtual ifaddrs* get() = 0;
    virtual bool set(std::string pIfname, std::string pIfaddress) = 0;
};

class Ifaddr4 : public IfaddrInterface
{
    struct sockaddr_in ifa_addr;      // network address
    struct sockaddr_in ifa_netmask;   // netmask
    struct sockaddr_in ifa_ifu;    // broadcast addr or point-to-point dest addr
    struct ifaddrs ifaddr;

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
    virtual ~Ifaddr4() {}

    Ifaddr4()
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


    ifaddrs* get() override
    // Return the pointer to the ifaddr structure
    {
        return &ifaddr;
    }


    bool set(std::string pIfname, std::string pIfaddress) override
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
        // broadcast = ip-addr or the inverted subnet-mask
        ifa_ifu.sin_addr.s_addr = ifa_addr.sin_addr.s_addr | ~ ifa_netmask.sin_addr.s_addr;

        return true;
    }
};
