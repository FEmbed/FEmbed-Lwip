/* X-Cheng LWIP Wrap Module Source
 * Copyright (c) 2018-2028 Gene Kong
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef FEMBED_LWIP_SRC_TCPCLIENT_H_
#define FEMBED_LWIP_SRC_TCPCLIENT_H_

#include "lwip/arch.h"
#include "lwip/sockets.h"
#if USE_ESPRESSIF8266
#undef connect
#undef write
#undef read
#undef bind
#undef send
#undef socket
#endif

#include <Client.h>

namespace FEmbed {

class TCPClient : public Client
{
 public:
    TCPClient();
    TCPClient(int sock_id, struct sockaddr_in &sa);
    virtual ~TCPClient();

    virtual int connect(IPAddress ip, uint16_t port);
    virtual int connectV4(u32_t ip, uint16_t port, int32_t timeout = 5000);
    virtual int connect(const char *host, uint16_t port);
    virtual int connect(const char *host, uint16_t port, int32_t timeout);
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *buf, size_t size);
    virtual size_t write(const char *msg);
    virtual int available();
    virtual int read();
    virtual int read(uint8_t *buf, size_t size);
    virtual int peek();
    virtual void flush();
    virtual void stop();
    virtual uint8_t connected();
    virtual operator bool();

    static int hostByName(const char* host, IPAddress& aResult);

    void setKeepAlive(int time);

 protected:
    int m_socket_fd;
    struct sockaddr_in m_sa;

    bool _connected;
};

} /* namespace FEmbed */

#endif /* FEMBED_LWIP_SRC_TCPCLIENT_H_ */
