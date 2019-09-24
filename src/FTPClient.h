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

#ifndef LIB_FE_LWIP_SRC_FTPCLIENT_H_
#define LIB_FE_LWIP_SRC_FTPCLIENT_H_

#include <memory>
using std::shared_ptr;

#ifndef FTPLIB_BUFSIZ
#define FTPLIB_BUFSIZ   (1024)
#endif

#ifndef FTPLIB_RESPONSE_BUFSIZ
#define FTPLIB_RESPONSE_BUFSIZ (1024)
#endif

#if USE_ESPRESSIF8266
#undef connect
#undef write
#undef read
#endif

namespace FEmbed {

class FTPClientBase;

typedef int (*FtpCallback)(FTPClientBase *nControl, uint32_t xfered, void *arg);

class FTPClientBase : 
    public std::enable_shared_from_this<FTPClientBase> {
public:
    FTPClientBase();
    /** Data transfer mode **/
    enum TransferMode {
        TEXT='A',
        BINARY= 'I'
    };
        /** Data connection type **/
    enum Type {
        DIR=1,
        DIR_VERBOSE,
        FILE_READ,
        FILE_WRITE
    };
    
    /** Connection mode **/
    enum Mode {
        PASSIVE=1,
        PORT
    };
    
    /** Close FTP connection **/
    virtual ~FTPClientBase();
    operator bool();

    int readLine(char *buf,int max);
    int writeLine(const char *buf, int len);
    int readResp(char c);
    int sendCmd(const char *cmd, char expresp);

    char *cput,*cget;
    int sock_fd;
    int cavail,cleft;
    char *buf;
    int dir;
    shared_ptr<FTPClientBase> ctrl;
    shared_ptr<FTPClientBase> data;    
    int cmode;
    struct timeval idletime;
    FtpCallback idlecb;
    void *idlearg;
    uint32_t xfered;
    uint32_t cbbytes;
    uint32_t xfered1;
    char response[FTPLIB_RESPONSE_BUFSIZ];
};

/** FTP main (control) connection **/
class FTPClient : public FTPClientBase
{
    friend class FTPClientData;
 public:
    FTPClient();
    FTPClient(const char * host);
    virtual ~FTPClient();

    bool connectHost(const char *host);

    /** Returns last response **/
    const char * getLastResponse();

    /** Set connection mode **/
    void setConnectionMode(Mode mode)
    {
        this->cmode = mode;
    }

    void setCallback(FtpCallback cb, void *data, int cbbytes)
    {
        this->idlecb = cb;
        this->idlearg = data;
        this->cbbytes = cbbytes;
    }

    void setIdleTime(int ms)
    {
        this->idletime.tv_sec = ms / 1000;
	    this->idletime.tv_usec = (ms % 1000) * 1000;
    }

    /** Log in to the server **/
    int login(const char * user, const char * password);
    /** Returns system type **/
    std::string getSystemType();
    /** Returns current directory **/
    std::string getDirectory();
    /** Change current directory **/
    int cd(const char * directory);
    /** Move up one level **/
    int cdup();
    /** Get terse directory listing **/
    int getList(const char * filename, const char * path);
    /** Get detailed directory listing **/
    int getFullList(const char * filename, const char * path);
    /** Make a directory **/
    int mkdir(const char * directory);
    /** Remove a directory **/
    int rmdir(const char * directory);
    /** Rename or move file **/
    int rename(const char * oldName, const char * newName);
    /** Delete a file **/
    int unlink(const char * filename);
    /** Download a file **/
    int get(const char * local, const char * remote, TransferMode mode);
    /** Upload a file **/
    int put(const char * local, const char * remote, TransferMode mode);
    /** Get file size **/
    unsigned size(const char * path, TransferMode mode);

    /** Get file modification date/time **/
    std::string modDate(const char * path);
    /** Run site-dependent command **/
    int site(const char * command);

private:
    int xfer(const char *localfile, const char *path, Type typ, TransferMode mode);
};

/** FTP data connection **/
class FTPClientData : public FTPClientBase {
public:
    /** Open FTP data connection **/
    FTPClientData(shared_ptr<FTPClientBase> ftp_cli, const char * path, Type type, TransferMode mode);
    ~FTPClientData();

    int openDataConn(shared_ptr<FTPClientBase> ftp_cli, TransferMode mode, int dir);
    int acceptConnection(shared_ptr<FTPClientBase> ftp_cli);

    /** Read data from a remote file or directory **/
    ssize_t read(void * buffer, size_t length);

    /** Write data to a remote file **/
    ssize_t write(void * buffer, size_t length);
};

} /* namespace FEmbed */

#endif /* LIB_FE_LWIP_SRC_FTPCLIENT_H_ */
