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
#include "Arduino.h"
#include <TCPServer.h>
#include "lwip/netdb.h"
#include <cstdio>

#ifndef TCP_SERVER_MAX_CONN
#define TCP_SERVER_MAX_CONN         (2)
#endif

namespace FEmbed {

TCPServer::TCPServer()
{
    m_socket_fd = -1;
}

TCPServer::TCPServer(uint16_t port)
{
    this->begin();
    this->establish(port);
}

TCPServer::~TCPServer()
{
    // TODO Auto-generated destructor stub
}

void TCPServer::begin()
{
    memset(&m_serv_addr, 0, sizeof(m_serv_addr));
    memset(&m_cli_addr, 0, sizeof(m_cli_addr));
    m_socket_fd = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket_fd == -1) {
        log_e("failed to create m_socket_fd!");
    }
}

void TCPServer::end()
{
    if(m_socket_fd >= 0)
    {
        lwip_close(m_socket_fd);
    }
}

int TCPServer::establish(uint16_t port, uint32_t bind_addr)
{
    int rc = 0;
    int opt_val = 1;
    struct timeval timeout = {5, 0};

    m_serv_addr.sin_family = AF_INET;
    m_serv_addr.sin_addr.s_addr = htonl(bind_addr);
    m_serv_addr.sin_port = htons(port);

    if(m_socket_fd < 0) return false;

    setsockopt(m_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);
    setsockopt(m_socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    rc = lwip_bind(m_socket_fd, (struct sockaddr *) &m_serv_addr, sizeof(m_serv_addr));
    if (rc < 0) {
        log_e("could not bind socket(0x%x)!", rc);
        return false;
    }

    rc = listen(m_socket_fd, TCP_SERVER_MAX_CONN);
    if (rc < 0) {
        log_e("Could not listen on socket(0x%x).", rc);
        return false;
    }

    log_d("Server is listening on %d.",port);
    return true;
}


std::shared_ptr<TCPClient> TCPServer::accept()
{
    socklen_t client_len;
    int sock_fd;
    if(m_socket_fd >= 0)
    {
        sock_fd = lwip_accept(m_socket_fd, (struct sockaddr *) &m_cli_addr, &client_len);
        if(sock_fd >= 0)
        {
            std::shared_ptr<TCPClient> ncli(new TCPClient(sock_fd, m_cli_addr));
            return ncli;
        }
    }
    return nullptr;
}

TCPClient TCPServer::available()
{
    socklen_t client_len;
    int sock_fd;
    if(m_socket_fd >= 0)
    {
        sock_fd = lwip_accept(m_socket_fd, (struct sockaddr *) &m_cli_addr, &client_len);
        if(sock_fd >= 0)
        {
            
            return TCPClient(sock_fd, m_cli_addr);
        }
    }
    return TCPClient();
}


} /* namespace FEmbed */
