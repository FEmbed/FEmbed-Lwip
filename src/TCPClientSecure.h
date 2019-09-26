/*
  TCPClientSecure.h - Base class that provides Client SSL to FEmbed
  Copyright (c) 2011 Adrian McEwen.  All right reserved.
  Additions Copyright (C) 2017 Evandro Luis Copercini.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef WiFiClientSecure_h
#define WiFiClientSecure_h
#include "Arduino.h"
#include "IPAddress.h"
#include "ssl_client.h"

#include <TCPClient.h>

namespace FEmbed {

class TCPClientSecure : public TCPClient
{
protected:
    sslclient_context *sslclient;
 
    int _lastError = 0;
	int _peek = -1;
    int _timeout = 0;
    const char *_CA_cert;
    const char *_cert;
    const char *_private_key;
    const char *_pskIdent; // identity for PSK cipher suites
    const char *_psKey; // key in hex for PSK cipher suites

public:
    TCPClientSecure *next;
    TCPClientSecure();
    TCPClientSecure(int socket);
    virtual ~TCPClientSecure();
    virtual int connect(IPAddress ip, uint16_t port);
    virtual int connect(IPAddress ip, uint16_t port, int32_t timeout);
    virtual int connect(const char *host, uint16_t port);
    virtual int connect(const char *host, uint16_t port, int32_t timeout);
    virtual int connect(IPAddress ip, uint16_t port, const char *rootCABuff, const char *cli_cert, const char *cli_key);
    virtual int connect(const char *host, uint16_t port, const char *rootCABuff, const char *cli_cert, const char *cli_key);
    virtual int connect(IPAddress ip, uint16_t port, const char *pskIdent, const char *psKey);
    virtual int connect(const char *host, uint16_t port, const char *pskIdent, const char *psKey);
	virtual int peek();
    virtual size_t write(uint8_t data);
    virtual size_t write(const uint8_t *buf, size_t size);
    virtual int available();
    virtual int read();
    virtual int read(uint8_t *buf, size_t size);
    virtual void flush() {}
    virtual void stop();
    virtual uint8_t connected();
    int lastError(char *buf, const size_t size);
    void setPreSharedKey(const char *pskIdent, const char *psKey); // psKey in Hex
    void setCACert(const uint8_t *rootCA, size_t len);
    void setCACert(const char *rootCA);
    void setCertificate(const char *client_ca);
    void setPrivateKey (const char *private_key);
    bool loadCACert(Stream& stream, size_t size);
    bool loadCertificate(Stream& stream, size_t size);
    bool loadPrivateKey(Stream& stream, size_t size);
    bool verify(const char* fingerprint, const char* domain_name);
    void setHandshakeTimeout(unsigned long handshake_timeout);

    int setTimeout(uint32_t seconds){ return 0; }

    operator bool()
    {
        return connected();
    }
    TCPClientSecure &operator=(const TCPClientSecure &other);
    bool operator==(const bool value)
    {
        return bool() == value;
    }
    bool operator!=(const bool value)
    {
        return bool() != value;
    }
    bool operator==(const TCPClientSecure &);
    bool operator!=(const TCPClientSecure &rhs)
    {
        return !this->operator==(rhs);
    };

    int socket()
    {
        return sslclient->socket = -1;
    }

private:
    char *_streamLoad(Stream& stream, size_t size);

    //friend class WiFiServer;
    using Print::write;
};
}

#endif /* _WIFICLIENT_H_ */
