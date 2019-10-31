# FEmbed-Lwip

This lwip C++ wrap for FEmbed platform

## LIWP Options Reqirement

1. #define LWIP_COMPAT_SOCKETS 0
2. #define CONFIG_LWIP_SO_RCVBUF 1

## Third-Library Port List

1. [Embedded MQTT C/C++ Client Libraries](https://www.eclipse.org/paho/clients/c/embedded);
2. [ftlib (C++ rewrite)](http://nbpfaus.net/~pfau/ftplib/)
3. [HttpClient (Port from arduino-esp32)](https://github.com/espressif/arduino-esp32/tree/master/libraries/HTTPClient)

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

### 5. HTTPClient

* For HTTPS connect please config `CONFIG_MBEDTLS_SSL_IN_CONTENT_LEN`,
`CONFIG_MBEDTLS_SSL_OUT_CONTENT_LEN` bigger for SSL handshake.

```cpp
const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIN+jCCC+KgAwIBAgITIAAGDxuOsc5CY0aDQgAAAAYPGzANBgkqhkiG9w0BAQsF\n" \
"ADCBizELMAkGA1UEBhMCVVMxEzARBgNVBAgTCldhc2hpbmd0b24xEDAOBgNVBAcT\n" \
"B1JlZG1vbmQxHjAcBgNVBAoTFU1pY3Jvc29mdCBDb3Jwb3JhdGlvbjEVMBMGA1UE\n" \
"CxMMTWljcm9zb2Z0IElUMR4wHAYDVQQDExVNaWNyb3NvZnQgSVQgVExTIENBIDIw\n" \
"HhcNMTkwNDMwMjA0ODAwWhcNMjEwNDMwMjA0ODAwWjAXMRUwEwYDVQQDEwx3d3cu\n" \
"YmluZy5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDhIlPb0iP7\n" \
"xRmUScK43QI7Ci/lvfMumWhRFAHcFzjIDHs74sq0B+ze8HW5PR6LWRe/d3yR5dC8\n" \
"7gQs0qXGitzsP9vWJcpwKV273tlnWiEfgZx5tvNCFdHOqoYoHL3a8zed/JkGTEeX\n" \
"ukGEX0TeBgCjcVTj5qRxJhjlWxs3AcB/q4f4vi3QG80TbSU2UO0lkvhvfs73C1jq\n" \
"i7Zspia/YsMqcQ6X+APAZ+4guKjQr5q32tzj2FGtJO6ZmZuNV9Wwb32891UhwZ3D\n" \
"2PrIcnCNlIQ1/Fah6im7Vc67qO2x/++r7gO7PtR8byCFnFuNUVQxhSIkCkj6FvPx\n" \
"cYefok0wJ0VRAgMBAAGjggnIMIIJxDCCAfQGCisGAQQB1nkCBAIEggHkBIIB4AHe\n" \
"AHUA7ku9t3XOYLrhQmkfq+GeZqMPfl+wctiDAMR7iXqo/csAAAFqcApGzAAABAMA\n" \
"RjBEAiAiASIs5j19VcTLbxcOGHQlIl62d3iy1FY8dnNq+6lebQIgchbSq2Qh78zs\n" \
"mmucyslucBycij/FYUe3F1lNpJiB9KsAdgBVgdTCFpA2AUrqC5tXPFPwwOQ4eHAl\n" \
"CBcvo6odBxPTDAAAAWpwCkgDAAAEAwBHMEUCIQDx6RqcvDdfIY9qdAuaRFBVvSHN\n" \
"ttpAzie3KP9AAiGvBwIgSMvjse/hJusDoRFnSTtX96ierTaqzQH4oDLLnW/Gwc0A\n" \
"dQBc3EOS/uarRUSxXprUVuYQN/vV+kfcoXOUsl7m9scOygAAAWpwCkbmAAAEAwBG\n" \
"MEQCICbYF6Lv93BFrwLguzmas/5gQ87fzRHkTaMxDSD7PlhRAiA/DXOeTcHaiUPQ\n" \
"WsKbJ/7x9EWKvVisqtQMnYk6cBxbBwB2AESUZS6w7s6vxEAH2Kj+KMDa5oK+2Msx\n" \
"tT/TM5a1toGoAAABanAKRtoAAAQDAEcwRQIgardRfR7bxwSGF212a603dXYz6O5z\n" \
"YHpPks8/RR/AMzQCIQD4VYDD+2zVDHEjz8elkKEzhgzTdOMtc1yYhCU+eHAGkjAn\n" \
"BgkrBgEEAYI3FQoEGjAYMAoGCCsGAQUFBwMCMAoGCCsGAQUFBwMBMD4GCSsGAQQB\n" \
"gjcVBwQxMC8GJysGAQQBgjcVCIfahnWD7tkBgsmFG4G1nmGF9OtggV2E0t9CgueT\n" \
"egIBZAIBHTCBhQYIKwYBBQUHAQEEeTB3MFEGCCsGAQUFBzAChkVodHRwOi8vd3d3\n" \
"Lm1pY3Jvc29mdC5jb20vcGtpL21zY29ycC9NaWNyb3NvZnQlMjBJVCUyMFRMUyUy\n" \
"MENBJTIwMi5jcnQwIgYIKwYBBQUHMAGGFmh0dHA6Ly9vY3NwLm1zb2NzcC5jb20w\n" \
"HQYDVR0OBBYEFDcHtZt8HkKSRO8ETiTkCLixB9PaMAsGA1UdDwQEAwIEsDCCBW0G\n" \
"A1UdEQSCBWQwggVgggx3d3cuYmluZy5jb22CEGRpY3QuYmluZy5jb20uY26CEyou\n" \
"cGxhdGZvcm0uYmluZy5jb22CCiouYmluZy5jb22CCGJpbmcuY29tghZpZW9ubGlu\n" \
"ZS5taWNyb3NvZnQuY29tghMqLndpbmRvd3NzZWFyY2guY29tghljbi5pZW9ubGlu\n" \
"ZS5taWNyb3NvZnQuY29tghEqLm9yaWdpbi5iaW5nLmNvbYINKi5tbS5iaW5nLm5l\n" \
"dIIOKi5hcGkuYmluZy5jb22CGGVjbi5kZXYudmlydHVhbGVhcnRoLm5ldIINKi5j\n" \
"bi5iaW5nLm5ldIINKi5jbi5iaW5nLmNvbYIQc3NsLWFwaS5iaW5nLmNvbYIQc3Ns\n" \
"LWFwaS5iaW5nLm5ldIIOKi5hcGkuYmluZy5uZXSCDiouYmluZ2FwaXMuY29tgg9i\n" \
"aW5nc2FuZGJveC5jb22CFmZlZWRiYWNrLm1pY3Jvc29mdC5jb22CG2luc2VydG1l\n" \
"ZGlhLmJpbmcub2ZmaWNlLm5ldIIOci5iYXQuYmluZy5jb22CECouci5iYXQuYmlu\n" \
"Zy5jb22CEiouZGljdC5iaW5nLmNvbS5jboIPKi5kaWN0LmJpbmcuY29tgg4qLnNz\n" \
"bC5iaW5nLmNvbYIQKi5hcHBleC5iaW5nLmNvbYIWKi5wbGF0Zm9ybS5jbi5iaW5n\n" \
"LmNvbYINd3AubS5iaW5nLmNvbYIMKi5tLmJpbmcuY29tgg9nbG9iYWwuYmluZy5j\n" \
"b22CEXdpbmRvd3NzZWFyY2guY29tgg5zZWFyY2gubXNuLmNvbYIRKi5iaW5nc2Fu\n" \
"ZGJveC5jb22CGSouYXBpLnRpbGVzLmRpdHUubGl2ZS5jb22CDyouZGl0dS5saXZl\n" \
"LmNvbYIYKi50MC50aWxlcy5kaXR1LmxpdmUuY29tghgqLnQxLnRpbGVzLmRpdHUu\n" \
"bGl2ZS5jb22CGCoudDIudGlsZXMuZGl0dS5saXZlLmNvbYIYKi50My50aWxlcy5k\n" \
"aXR1LmxpdmUuY29tghUqLnRpbGVzLmRpdHUubGl2ZS5jb22CCzNkLmxpdmUuY29t\n" \
"ghNhcGkuc2VhcmNoLmxpdmUuY29tghRiZXRhLnNlYXJjaC5saXZlLmNvbYIVY253\n" \
"ZWIuc2VhcmNoLmxpdmUuY29tggxkZXYubGl2ZS5jb22CDWRpdHUubGl2ZS5jb22C\n" \
"EWZhcmVjYXN0LmxpdmUuY29tgg5pbWFnZS5saXZlLmNvbYIPaW1hZ2VzLmxpdmUu\n" \
"Y29tghFsb2NhbC5saXZlLmNvbS5hdYIUbG9jYWxzZWFyY2gubGl2ZS5jb22CFGxz\n" \
"NGQuc2VhcmNoLmxpdmUuY29tgg1tYWlsLmxpdmUuY29tghFtYXBpbmRpYS5saXZl\n" \
"LmNvbYIObG9jYWwubGl2ZS5jb22CDW1hcHMubGl2ZS5jb22CEG1hcHMubGl2ZS5j\n" \
"b20uYXWCD21pbmRpYS5saXZlLmNvbYINbmV3cy5saXZlLmNvbYIcb3JpZ2luLmNu\n" \
"d2ViLnNlYXJjaC5saXZlLmNvbYIWcHJldmlldy5sb2NhbC5saXZlLmNvbYIPc2Vh\n" \
"cmNoLmxpdmUuY29tghJ0ZXN0Lm1hcHMubGl2ZS5jb22CDnZpZGVvLmxpdmUuY29t\n" \
"gg92aWRlb3MubGl2ZS5jb22CFXZpcnR1YWxlYXJ0aC5saXZlLmNvbYIMd2FwLmxp\n" \
"dmUuY29tghJ3ZWJtYXN0ZXIubGl2ZS5jb22CE3dlYm1hc3RlcnMubGl2ZS5jb22C\n" \
"FXd3dy5sb2NhbC5saXZlLmNvbS5hdYIUd3d3Lm1hcHMubGl2ZS5jb20uYXUwgawG\n" \
"A1UdHwSBpDCBoTCBnqCBm6CBmIZLaHR0cDovL21zY3JsLm1pY3Jvc29mdC5jb20v\n" \
"cGtpL21zY29ycC9jcmwvTWljcm9zb2Z0JTIwSVQlMjBUTFMlMjBDQSUyMDIuY3Js\n" \
"hklodHRwOi8vY3JsLm1pY3Jvc29mdC5jb20vcGtpL21zY29ycC9jcmwvTWljcm9z\n" \
"b2Z0JTIwSVQlMjBUTFMlMjBDQSUyMDIuY3JsME0GA1UdIARGMEQwQgYJKwYBBAGC\n" \
"NyoBMDUwMwYIKwYBBQUHAgEWJ2h0dHA6Ly93d3cubWljcm9zb2Z0LmNvbS9wa2kv\n" \
"bXNjb3JwL2NwczAfBgNVHSMEGDAWgBSRnjtEbD1XnEJ3KjTXT9HMSpcs2jAdBgNV\n" \
"HSUEFjAUBggrBgEFBQcDAgYIKwYBBQUHAwEwDQYJKoZIhvcNAQELBQADggIBAA1g\n" \
"NJF5ks5Qrg0/qeOXQbcO3SCs+HKTKxVL8QdaTL3s5gsmWQzcYNS671DmN4lEob2g\n" \
"WWZKyKAzQbjDOcf9ndxX4+i+PaCw5K3uONbMOwnuOCwRvDy8YEoCb3OzKFX4sjzh\n" \
"1HVL/ljKHUPT+9ap/SpYserNxixibqF2LZYx+9hwr1bcx9GWrg3CoFUFgSZqRQ14\n" \
"eiK94iM5kzJLKynKPhez+UOwS5VRev1mxh5nD9hBPzXHHqI9mNWu/lyr7KPUMigi\n" \
"QfKKZuqV6W3i1H3BoJi1uDkL3SJo1F39XN3AyGSAZWS9RNn5JzEQQGiJRjrz/PE1\n" \
"vTg1BlbsPdKa4gGZGdGBWcj2eXZc+GbLpTy3qWlmJrEn2KGLeomyndlftRPFrBUH\n" \
"/5Mio5OeSawjlacBV25fKaoZ1BPc3i+HGKd5ctddCy6kJsgdMD221zGvf/0uW25Z\n" \
"ImzDeH7KkOcGbzyWJwBzDgra0RP+qRgK3aYPSWI81OLlnHJ2VOix/UU63NCK2fO/\n" \
"URzE8KxoHrgRGXCE52viHv6ksL7QXWelbERU7GEpcZU1suPhDohn4CrfrCYCjpa5\n" \
"Ys6ci7Rren82SsXJBfNrgm2U4lxWfzWj+2Ay6yATbdoOPntue8cbbMoTzoNMHQXD\n" \
"2DpjtFPs8/RVOFQb0IFVluCrTAnHmI8tTtsmzg6z\n" \
"-----END CERTIFICATE-----\n";


void configTime(const char* server1, const char* server2, const char* server3)
{
    if(sntp_enabled()){
        sntp_stop();
    }
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, (char*)server1);
    sntp_setservername(1, (char*)server2);
    sntp_setservername(2, (char*)server3);
    sntp_init();
}

void setClock() {
  configTime("cn.pool.ntp.org", "pool.ntp.org", "time.nist.gov");

  std::printf(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    std::printf(F("."));
    yield();
    nowSecs = time(nullptr);
  }

  printf("\n");
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  printf(F("Current time: "));
  printf(asctime(&timeinfo));
}

void test_http_client()
{
    shared_ptr<FEmbed::TCPClientSecure> scli(new FEmbed::TCPClientSecure());
    shared_ptr<FEmbed::HTTPClient> http(new FEmbed::HTTPClient());
    setClock();

    http->begin("http://example.com/index.html");
    std::printf("[HTTP] GET...\n");
    int httpCode = http->GET();

    // httpCode will be negative on error
    if(httpCode > 0) {
        std::printf("[HTTP] GET... code: %d\n", httpCode);

        if(httpCode == FEmbed::HTTP_CODE_OK) {
            String payload = http->getString();
            std::printf("%s", payload.c_str());
        }
    } else {
        std::printf("[HTTP] GET... failed, error: %s\n", http->errorToString(httpCode).c_str());
    }
    http->end();
#if 0
    if(scli)
        scli->setCACert(rootCACertificate);
    //if (http->begin(scli, "https://jigsaw.w3.org/HTTP/connection.html"))
    if (http->begin(scli, "https://bing.com"))
    {
        std::printf("[HTTPS] GET...\n");
        int httpCode = http->GET();

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            std::printf("[HTTPS] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == FEmbed::HTTP_CODE_OK || httpCode == FEmbed::HTTP_CODE_MOVED_PERMANENTLY) {
                String payload = http->getString();
                std::printf("%s", payload.c_str());
            }
        } else {
            std::printf("[HTTPS] GET... failed, error: %s\n", http->errorToString(httpCode).c_str());
        }
        http->end();
    }
    else {
        std::printf("[HTTPS] Unable to connect\n");
    }
#endif
}
```