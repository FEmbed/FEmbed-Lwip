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

#include "TCPClient.h"
#include "lwip/netdb.h"
#include <cstdio>

#ifdef  LOG_TAG
    #undef  LOG_TAG
#endif
#define LOG_TAG                             "TCPClient"

#define TCP_RAW_DEBUG           0

namespace FEmbed {

TCPClient::TCPClient()
{
    m_socket_fd = -1;
}

TCPClient::TCPClient(int sock_id, struct sockaddr_in &sa)
{
    m_socket_fd = sock_id;
    memcpy(&m_sa, &sa, sizeof(sa));
}

TCPClient::~TCPClient()
{
    if(m_socket_fd >= 0)
    {
        lwip_shutdown(m_socket_fd, 2);
        lwip_close(m_socket_fd);
    }
}

int TCPClient::connectV4(u32_t ip, uint16_t port)
{
    int rc = 0;
    int on = 1;
    int time = 5*1000;
    if(m_socket_fd >= 0)
        this->stop();
    
    m_socket_fd = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(m_socket_fd < 0)
    {
        log_e("socket call failed");
        return -1;
    }
    if(lwip_setsockopt(m_socket_fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&time, sizeof(time)) == -1)
    {
        log_e("setsockopt SO_KEEPALIVE failed!");
	    lwip_close(m_socket_fd);
        return -1;
    }
    if (lwip_setsockopt(m_socket_fd,SOL_SOCKET,SO_REUSEADDR, (void *) &on, sizeof(on)) == -1)
    {
	    log_e("setsockopt SO_REUSEADDR failed!");
	    lwip_close(m_socket_fd);
	    return -1;
    }

    memset(&m_sa, 0, sizeof(sockaddr_in));
    m_sa.sin_family = AF_INET;
    m_sa.sin_addr.s_addr = ip;
    m_sa.sin_port = lwip_htons(port);

    if((rc = lwip_connect(m_socket_fd, (sockaddr *)&m_sa, sizeof(m_sa))) < 0)
    {

        log_e("connect to 0x%08x:%d failed(%d).", ip, port, rc);
        lwip_close(m_socket_fd);
        m_socket_fd =  -2;
    }

    return m_socket_fd;
}
int TCPClient::connect(IPAddress ip, uint16_t port)
{
    return this->connectV4(ip.v4(), port);
}

int TCPClient::connect(const char *host, uint16_t port)
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

void TCPClient::setKeepAlive(int time)
{
    if(m_socket_fd >= 0)
    {
        lwip_setsockopt(m_socket_fd,
                        SOL_SOCKET,
                        SO_KEEPALIVE,
                        (char*)&time, sizeof(time));
    }
}

size_t TCPClient::write(uint8_t c)
{
    if(m_socket_fd < 0)
        return -1;
#if TCP_RAW_DEBUG
    log_d("> %02x >", c);
#endif
    int rc = lwip_write(m_socket_fd, &c, 1);
    return rc;
}

size_t TCPClient::write(const uint8_t *buf, size_t size)
{
    if(m_socket_fd < 0)
        return -1;
#if TCP_RAW_DEBUG
    std::printf("> ");
    for(size_t i=0; i< size; i++)
        std::printf("%02x ", buf[i]);
    log_d(">");
#endif
    int rc = lwip_write(m_socket_fd, buf, size);
    return rc;
}

int TCPClient::available()
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

int TCPClient::read()
{
    int rc = 0;
    char buf = 0;
    if(m_socket_fd < 0)
    {
        log_w("Read when no socket.");
        return-1;
    }

    if(lwip_recv(m_socket_fd, &buf, 1, 0))
    {
#if TCP_RAW_DEBUG
        log_d("< %02x <", buf);
#endif
        rc = buf;
    }
    return rc;
}

int TCPClient::read(uint8_t *buf, size_t size)
{
    if(m_socket_fd < 0)
    {
        log_w("Read %d when no socket.", size);
        return -1;
    }
    int rc = lwip_recv(m_socket_fd, buf, size, 0);
#if TCP_RAW_DEBUG
    std::printf("< ");
    for(int i=0; i< rc; i++)
        std::printf("%02x ", buf[i]);
    log_d("<");
#endif
    return rc;
}

int TCPClient::peek()
{
    return 0;
}

void TCPClient::flush()
{
    ///< nothing to do.
}

void TCPClient::stop()
{
    if(m_socket_fd >= 0)
    {
        lwip_shutdown(m_socket_fd, 2);
        lwip_close(m_socket_fd);
    }
    m_socket_fd = -1;
}

uint8_t TCPClient::connected()
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

TCPClient::operator bool()
{
    if(m_socket_fd >= 0) return true;
    return false;
}

} /* namespace FEmbed */
