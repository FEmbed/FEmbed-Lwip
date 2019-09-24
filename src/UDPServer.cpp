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
#include <UDPServer.h>
#include "lwip/netdb.h"
#include <cstdio>

#ifdef  LOG_TAG
    #undef  LOG_TAG
#endif
#define LOG_TAG                             "UDPServer"

#define UDP_RAW_DEBUG           0

namespace FEmbed {

UDPServer::UDPServer()
{
    m_socket_fd = -1;
}

UDPServer::~UDPServer()
{
    if(m_socket_fd >= 0)
    {
        lwip_close(m_socket_fd);
    }
}

void UDPServer::begin()
{
    memset(&m_serv_addr, 0, sizeof(m_serv_addr));
    memset(&m_cli_addr, 0, sizeof(m_cli_addr));
    m_socket_fd = lwip_socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket_fd == -1) {
        log_e("failed to create m_socket_fd!");
    }
}

void UDPServer::end()
{
    if(m_socket_fd >= 0)
    {
        lwip_close(m_socket_fd);
    }
    m_socket_fd = -1;
}

int UDPServer::establish(uint16_t port, uint32_t bind_addr)
{
    int rc = 0;
    uint32_t namelen;
    m_serv_addr.sin_family = AF_INET;
    m_serv_addr.sin_addr.s_addr = htonl(bind_addr);
    m_serv_addr.sin_port = htons(port);

    m_cli_addr.sin_family = AF_INET;
    m_cli_addr.sin_addr.s_addr = IPADDR_BROADCAST;
    m_cli_addr.sin_port = htons(port);

    struct timeval timeout = {5, 0};

    if(m_socket_fd < 0) return false;

    setsockopt(m_socket_fd,
               SOL_SOCKET, SO_RCVTIMEO,
               &timeout, sizeof(timeout));
    rc = bind(m_socket_fd,
               (struct sockaddr *)&m_serv_addr,
               sizeof(m_serv_addr));
    if (rc == -1) {
        log_e("failed to bind m_socket_fd to server port!");
    }
    else
    {
         /* Find out what port was really assigned and print it */
        namelen = sizeof(m_serv_addr);
        if ((rc = getsockname(m_socket_fd, (struct sockaddr *) &m_serv_addr, &namelen)) < 0)
        {
           log_e("getsockname failed!");
        }

        log_d("port assigned is %d.\n", ntohs(m_serv_addr.sin_port));
    }
    return rc;
}

size_t UDPServer::write(uint8_t c)
{
    int rc = 0;
    uint32_t namelen = sizeof(m_cli_addr);
    if((rc = lwip_sendto(m_socket_fd, &c, 1, 0,
                         (struct sockaddr *) &m_cli_addr, namelen)) < 0)
    {
        log_e("Sendto 0x%08x failed!", m_cli_addr.sin_addr.s_addr);
    }
    return rc;
}

size_t UDPServer::write(const uint8_t *buf, size_t size)
{
    int rc = 0;
    uint32_t namelen = sizeof(m_cli_addr);
    if((rc = lwip_sendto(m_socket_fd, buf, size, 0,
                         (struct sockaddr *) &m_cli_addr, namelen)) < 0)
    {
        log_e("Sendto 0x%08x failed!", m_cli_addr.sin_addr.s_addr);
    }
    return rc;
}

int UDPServer::read()
{
    int rc = 0;
    uint8_t c;
    uint32_t namelen = sizeof(m_cli_addr);
    if((rc = lwip_recvfrom(m_socket_fd, (void *)&c, 1, 0,
                         (struct sockaddr *) &m_cli_addr, &namelen)) < 0)
    {
    }
    return rc;
}

int UDPServer::read(uint8_t *buf, size_t size)
{
    int rc = 0;
    uint32_t namelen = sizeof(m_cli_addr);
    if((rc = lwip_recvfrom(m_socket_fd, (void *)buf, size, 0,
                         (struct sockaddr *) &m_cli_addr, &namelen)) < 0)
    {
    }
    return rc;
}

} /* namespace FEmbed */
