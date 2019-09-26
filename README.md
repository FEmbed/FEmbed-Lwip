# FEmbed-Lwip

This lwip C++ wrap for FEmbed platform

## LIWP Options Reqirement

1. #define LWIP_COMPAT_SOCKETS 0
2. #define CONFIG_LWIP_SO_RCVBUF 1

## Third-Library Port List

1. [Embedded MQTT C/C++ Client Libraries](https://www.eclipse.org/paho/clients/c/embedded);
2. [ftlib (C++ rewrite)](http://nbpfaus.net/~pfau/ftplib/)

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

### 5. FTPClient

This code is refer from ftplib, but re-write by C++.

* Current version no PORT mode support.
* Use FEmbed::FTPClientDataCallback to process file operate in Embed system.

```cpp
const char * test_fs_file = "This is virtual file!";
class TestFSCallback : public FEmbed::FTPClientDataCallback
{
    public:
        virtual void open(const char *filename, const char* mode) {
            (void) filename;
            (void) mode;
            ptr = 0;
        }

        virtual void close()
        {
            ptr = 0;
        }
        virtual size_t read(uint8_t *buf, size_t size)
        {
            if(ptr >= strlen(test_fs_file))
                return 0;
            size_t i = 0;
            for(size_t j = ptr; i< size && j < strlen(test_fs_file); i++, j++)
            {
                buf[i] = test_fs_file[j];
            }
            ptr +=i;
            return i;
        }

        virtual size_t write(uint8_t *buf, size_t size)
        {
            std::printf("emu ftp write: \n");
            for(size_t i = 0; i< size; i++)
                std::printf("%c", buf[i]);
            return size;
        }
        uint32_t ptr;
};

void test_ftp_client()
{
    shared_ptr<FEmbed::FTPClient> ftp_cli(new FEmbed::FTPClient());
    shared_ptr<FEmbed::FTPClientDataCallback> fs_cb(new TestFSCallback());
    ftp_cli->connectHost("10.0.0.114:21");
    ftp_cli->setFSCallback(fs_cb);
    if(*ftp_cli)
    {
        ftp_cli->login();
        log_d("PWD: %s", ftp_cli->getDirectory().c_str());
        log_d("SYST: %s", ftp_cli->getSystemType().c_str());
        ftp_cli->getList(NULL, "/");
        ftp_cli->getFullList(NULL, "/");
        ftp_cli->size("Test.txt");
        ftp_cli->modDate("Test.txt");
        ftp_cli->mkdir("test");
        ftp_cli->cd("test");
        ftp_cli->cdup();
        ftp_cli->rmdir("error");        ///Failed
        ftp_cli->rmdir("test");
        ftp_cli->put(NULL, "put.txt");
        ftp_cli->put(NULL, "put.bin", FEmbed::FTPClient::BINARY);
        ftp_cli->get(NULL, "Test.txt");
        ftp_cli->get(NULL, "Test.???"); ///Failed
        ftp_cli->rename("put.txt", "new.txt");
        ftp_cli->unlink("new.txt");
        ftp_cli->unlink("put.bin");
    }
}
```
