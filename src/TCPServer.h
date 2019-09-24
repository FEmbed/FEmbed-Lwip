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

#ifndef LIB_FE_LWIP_SRC_TCPSERVER_H_
#define LIB_FE_LWIP_SRC_TCPSERVER_H_
#include <memory>

#include "Server.h"
#include "TCPClient.h"
#include "lwip/sockets.h"

#if USE_ESPRESSIF8266
#undef connect
#undef write
#undef read
#undef accept
#endif

namespace FEmbed {

class TCPServer
{
 public:
    TCPServer();
    virtual ~TCPServer();

    virtual void begin();
    virtual void end();

    /**
     * establish connect at bind address.
     * @param port bind server port.
     * @param bind_addr bind_addr use MCU endian.
     * @return establish connect or not.
     */
    virtual int establish(uint16_t port, uint32_t bind_addr = INADDR_ANY);
    std::shared_ptr<TCPClient> accept();

 private:
    int m_socket_fd;
    struct sockaddr_in m_serv_addr, m_cli_addr;
};

} /* namespace FEmbed */

#endif /* LIB_FE_LWIP_SRC_TCPSERVER_H_ */
