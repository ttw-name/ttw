#ifndef __TTW_ADDRESS_H
#define __TTW_ADDRESS_H

#include <memory>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

namespace ttw {

    class IPv4Address{
    public:
        using ptr = std::shared_ptr<IPv4Address>;
        IPv4Address(sockaddr_in& address);
        IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);
        IPv4Address(const std::string address, uint16_t port = 0);


        int getFamiy();

        const sockaddr* getAddress() const;
        sockaddr* getAddress();
        socklen_t getAddressLen() const;

        std::string toString() const;


        uint32_t getPort() const;
        void setPort(uint16_t port);


    private:
        sockaddr_in m_addr;

    };
}


#endif