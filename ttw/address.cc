
#include <cstring>
#include <sstream>
#include "address.h"
#include "swapendian.h"
#include "config.h"
static auto logger_ = ttw::LogManage::GetLogManage()->getLogger("root");
namespace ttw {
    IPv4Address::IPv4Address(sockaddr_in& address){
        m_addr = address;
    }
    IPv4Address::IPv4Address(uint32_t address, uint16_t port){
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin_family = AF_INET;
        m_addr.sin_port = bytesOnBigSwapEndin(port);
        m_addr.sin_addr.s_addr = bytesOnBigSwapEndin(address);
    }
    IPv4Address::IPv4Address(const std::string address, uint16_t port){
        uint32_t ip_num;
        if(inet_pton(AF_INET, address.c_str(), &ip_num) == 0){
            LOG_ERROR(logger_) << "IP address invalid!";
        }
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin_family = AF_INET;
        m_addr.sin_port = bytesOnBigSwapEndin(port);
        m_addr.sin_addr.s_addr = ip_num;
    }


    int IPv4Address::getFamiy(){
        return m_addr.sin_family;
    }

    const sockaddr* IPv4Address::getAddress() const {
        return (sockaddr* )&m_addr;
    }
    sockaddr* IPv4Address::getAddress(){
        return (sockaddr* )&m_addr;
    }
    socklen_t IPv4Address::getAddressLen() const {
        return sizeof(m_addr);
    }

    std::string IPv4Address::toString() const {
        uint32_t addr = bytesOnBigSwapEndin(m_addr.sin_addr.s_addr);
        std::stringstream os;
        os  << ((addr >> 24) & 0xff ) << "."
            << ((addr >> 16) & 0xff ) << "."
            << ((addr >>  8) & 0xff ) << "."
            << (addr         & 0xff ) << ":";
        os << bytesOnBigSwapEndin(m_addr.sin_port);
        return os.str();
    }


    uint32_t IPv4Address::getPort() const{
        return bytesOnBigSwapEndin(m_addr.sin_port);

    }
    void IPv4Address::setPort(uint16_t port){
        m_addr.sin_port = bytesOnBigSwapEndin(port);
    }
}