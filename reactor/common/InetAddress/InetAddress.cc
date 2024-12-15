#include "InetAddress.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>  // bzero
//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

using namespace reactor;

static const in_addr_t kInaddrAny = INADDR_ANY;

InetAddress::InetAddress(uint16_t port) {
  bzero(&addr_, sizeof addr_);
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr = htonl(kInaddrAny);
  addr_.sin_port = htons(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port) {
  bzero(&addr_, sizeof addr_);
  addr_.sin_family = AF_INET;
  addr_.sin_port = htons(port);
  if (::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0) {
    // LOG_ERROR << "inet_pton err";
  }
}

std::string InetAddress::toHostPort() const {
  char buf[32];
  char host[INET_ADDRSTRLEN] = "INVALID";
  ::inet_ntop(AF_INET, &addr_.sin_addr, host, sizeof host);
  uint16_t port = ntohs(addr_.sin_port);
  snprintf(buf, sizeof buf, "%s:%u", host, port);
  return buf;
}
