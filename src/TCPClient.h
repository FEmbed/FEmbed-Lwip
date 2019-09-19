/*
 * TCPClient.h
 *
 *  Created on: 2019年9月19日
 *      Author: Gene Kong
 */

#ifndef FEMBED_LWIP_SRC_TCPCLIENT_H_
#define FEMBED_LWIP_SRC_TCPCLIENT_H_

#include <Client.h>
#include "lwip/sockets.h"

#if USE_ESPRESSIF8266
#undef connect
#undef write
#undef read
#endif

namespace FEmbed {

class TCPClient : public Client
{
 public:
    TCPClient();
    virtual ~TCPClient();

    virtual int connect(IPAddress ip, uint16_t port);
    virtual int connect(const char *host, uint16_t port);
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *buf, size_t size);
    virtual int available();
    virtual int read();
    virtual int read(uint8_t *buf, size_t size);
    virtual int peek();
    virtual void flush();
    virtual void stop();
    virtual uint8_t connected();
    virtual operator bool();

    void setKeepAlive(int time);
 private:
    int connectV4(u32_t ip, uint16_t port);

    int m_socket_fd;
    sockaddr_in m_sa, m_ra;
};

} /* namespace FEmbed */

#endif /* FEMBED_LWIP_SRC_TCPCLIENT_H_ */
