# FEmbed-Lwip

This lwip C++ wrap for FEmbed platform

## LIWP Options Reqirement

1. #define LWIP_COMPAT_SOCKETS 0
2. #define CONFIG_LWIP_SO_RCVBUF 1

## Third-Library Port List

1. [Embedded MQTT C/C++ Client Libraries](https://www.eclipse.org/paho/clients/c/embedded);
2. [FTP-Client(Port to the lwip)](https://github.com/anhthii/FTP-Client)

## Usage

### 1. TCPClient

```cpp
void tcp_client_test()
{
    int rc;
    std::shared_ptr<FEmbed::TCPClient> tcp_cli(new FEmbed::TCPClient());
    if(tcp_cli->connect("10.0.0.114", 1234) >=0)
    {
        tcp_cli->print("Hello world!");
    }
    else
    {
        log_e("TCP Client test failed.");
    }
}
```

### 2. TCPServer

```cpp
uint8_t test_buff[256];
void tcp_server_test()
{
    int rc;
    std::shared_ptr<FEmbed::TCPServer> tcp_ser(new FEmbed::TCPServer());
    tcp_ser->begin();
    if(tcp_ser->establish(4889) >= 0)
    {
        do {
            auto cli = tcp_ser->accept();
            if(cli)
            {
                rc = cli->read(test_buff, 256);
                break;
            }
        } while(1);
        log_d("TCP received %s", test_buff);
    }
    tcp_ser->end();
}
```

### 3. UDPClient

```cpp
void udp_client_test()
{
    int rc;
    std::shared_ptr<FEmbed::UDPClient> udp_cli(new FEmbed::UDPClient());
    if(udp_cli->connect("10.0.0.114", 1234) >=0)
    {
        udp_cli->print("Hello world!");
    }
    else
    {
        log_e("UDP Client test failed.");
    }
}
```

### 4. UDPServer

```cpp
uint8_t test_buff[256];
void udp_server_test()
{
    int rc;
    std::shared_ptr<FEmbed::UDPServer> udp_ser(new FEmbed::UDPServer());
    udp_ser->begin();
    if(udp_ser->establish(48899) >= 0)
    {
        do {
            rc = udp_ser->read(test_buff, 256);
            log_d("udp read rc %d.", rc);
        } while(rc < 0);
        log_d("UDP received %s", test_buff);
    }
    udp_ser->end();
}
```
