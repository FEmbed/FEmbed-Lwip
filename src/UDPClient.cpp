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

#include <UDPClient.h>
#include "lwip/netdb.h"
#include <cstdio>

#ifdef  LOG_TAG
    #undef  LOG_TAG
#endif
#define LOG_TAG                             "UDPClient"

#define UDP_RAW_DEBUG           0

namespace FEmbed {

UDPClient::UDPClient()
{
    m_socket_fd = -1;
}

UDPClient::~UDPClient()
{
    if(m_socket_fd >= 0)
        lwip_close(m_socket_fd);
}

int UDPClient::connectV4(u32_t ip, uint16_t port, int32_t timeout)
{
    int rc = 0;
    m_socket_fd = lwip_socket(AF_INET, SOCK_DGRAM, 0);
    if(m_socket_fd < 0)
    {
        log_e("socket call failed");
        return -1;
    }
    
    if(lwip_setsockopt(m_socket_fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&timeout, sizeof(timeout)) == -1)
    {
        log_e("setsockopt SO_KEEPALIVE failed!");
	    lwip_close(m_socket_fd);
        return -1;
    }

    memset(&m_sa, 0, sizeof(sockaddr_in));
    m_sa.sin_family = AF_INET;
    m_sa.sin_port = lwip_htons(port);
    m_sa.sin_addr.s_addr = ip;

    return m_socket_fd;
}
int UDPClient::connect(IPAddress ip, uint16_t port)
{
    return this->connectV4(ip.v4(), port, 5000);
}

int UDPClient::connect(const char *host, uint16_t port)
{
    return this->connect(host, port);
}

int UDPClient::connect(const char *host, uint16_t port, int32_t timeout)
{
    if(m_socket_fd >= 0) return m_socket_fd;

    struct addrinfo *result = NULL;
    struct addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};
    uint32_t ip = inet_addr(host);
    if(ip == IPADDR_NONE)   //< Check for this host is domain.
    {
        if (lwip_getaddrinfo(host, NULL, &hints, &result) == 0)
        {
            struct addrinfo* res = result;
            /* prefer ip4 addresses */
            while (res)
            {
                if (res->ai_family == AF_INET)
                {
                    result = res;
                    break;
                }
                res = res->ai_next;
            }
            if (result->ai_family == AF_INET)
            {
                ip = ((struct sockaddr_in*)(result->ai_addr))->sin_addr.s_addr;
            }
            lwip_freeaddrinfo(result);
        }
    }
    if(ip == IPADDR_NONE)
    {
        log_e("Can't parse right IP address for host:%s.", host);
        return -1;
    }
    return this->connectV4(ip, port);
}

size_t UDPClient::write(uint8_t c)
{
    if(m_socket_fd < 0)
        return -1;
#if UDP_RAW_DEBUG
    log_d("> %02x >", c);
#endif
    int rc = lwip_sendto(m_socket_fd, &c, 1,
                         0,
                         (sockaddr *)&m_sa, sizeof(m_sa));
    return rc<0?0:rc;
}

size_t UDPClient::write(const uint8_t *buf, size_t size)
{
    if(m_socket_fd < 0)
        return -1;
#if UDP_RAW_DEBUG
    std::printf("> ");
    for(size_t i=0; i< size; i++)
        std::printf("%02x ", buf[i]);
    log_d(">");
#endif
    int rc = lwip_sendto(m_socket_fd, buf, size,
                         0,
                         (sockaddr *)&m_sa, sizeof(m_sa));
    return rc<0?0:rc;
}

int UDPClient::available()
{
    int rc = 0;
    if(m_socket_fd >= 0)
    {
        lwip_ioctl(m_socket_fd, FIONREAD, &rc);
        if(rc <0)
            rc = 0;
    }
    return rc;
}

int UDPClient::read()
{
    int rc = 0;
    char buf = 0;
    if(m_socket_fd < 0)
    {
        log_w("Read when no socket.");
        return -1;
    }

    if(lwip_recv(m_socket_fd, &buf, 1, 0))
    {
#if UDP_RAW_DEBUG
        log_d("< %02x <", buf);
#endif
        rc = buf;
    }
    return rc;
}

int UDPClient::read(uint8_t *buf, size_t size)
{
    if(m_socket_fd < 0)
    {
        log_w("Read %d when no socket.", size);
        return -1;
    }
    int rc = lwip_recv(m_socket_fd, buf, size, 0);
#if UDP_RAW_DEBUG
    std::printf("< ");
    for(int i=0; i< rc; i++)
        std::printf("%02x ", buf[i]);
    log_d("<");
#endif
    return rc;
}

int UDPClient::peek()
{
    return 0;
}

void UDPClient::flush()
{
    ///< nothing to do.
}

void UDPClient::stop()
{
    if(m_socket_fd >= 0)
    {
        lwip_close(m_socket_fd);
    }
    m_socket_fd = -1;
}

uint8_t UDPClient::connected()
{
    if(m_socket_fd < 0)
        return 0;

    int error_code;
    int error_code_size = sizeof(error_code);
    lwip_getsockopt(m_socket_fd, SOL_SOCKET,
                    SO_ERROR, &error_code,
                    (socklen_t*) &error_code_size);
    if(error_code == 0)
        return 1;
    return 0;
}

UDPClient::operator bool()
{
    if(m_socket_fd >= 0) return true;
    return false;
}

} /* namespace FEmbed */
