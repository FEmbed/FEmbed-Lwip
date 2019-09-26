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

#include "TCPClient.h"
#include <sys/time.h>
#include <memory>
#include <cstdio>
using std::shared_ptr;

#ifndef FTPLIB_BUFSIZ
#define FTPLIB_BUFSIZ   (512)
#endif

#ifndef FTPLIB_RESPONSE_BUFSIZ
#define FTPLIB_RESPONSE_BUFSIZ (512)
#endif

namespace FEmbed {

class FTPClientDataCallback 
{
    public:
        virtual ~FTPClientDataCallback() {}
        virtual void open(const char *filename, const char* mode)  = 0;
        virtual size_t read(uint8_t *buf, size_t size) = 0;
        virtual size_t write(uint8_t *buf, size_t size) = 0;
        virtual void close() = 0;
};

class FTPClientBase : 
    public TCPClient,
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

    int readLine(char *buf,int max);
    int writeLine(const char *buf, int len);
    int readResp(char c);

    char *cput,*cget;
    int cavail,cleft;
    int dir;
    shared_ptr<FTPClientBase> ctrl;
    shared_ptr<FTPClientBase> data;    
    int cmode;
    struct timeval idletime;
    uint32_t xfered;
    uint32_t xfered1;
    char buf[FTPLIB_BUFSIZ];
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

    void setFSCallback(shared_ptr<FTPClientDataCallback> fs_cb)
    {
        m_fs_cb = fs_cb;
    }

    /** Set connection mode **/
    void setConnectionMode(Mode mode)
    {
        this->cmode = mode;
    }

    void setIdleTime(int ms)
    {
        this->idletime.tv_sec = ms / 1000;
	    this->idletime.tv_usec = (ms % 1000) * 1000;
    }

    /** Log in to the server **/
    int login(const char * user = "anonymous", const char * password = NULL);
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
    int get(const char * local, const char * remote, TransferMode mode = TEXT);
    /** Upload a file **/
    int put(const char * local, const char * remote, TransferMode mode = TEXT);
    /** Get file size **/
    unsigned size(const char * path, TransferMode mode = TEXT);
    /** Get file modification date/time **/
    std::string modDate(const char * path);
    /** Run site-dependent command **/
    int site(const char * command);

private:
    int xfer(const char *localfile, const char *path, Type typ, TransferMode mode);
    shared_ptr<FTPClientDataCallback> m_fs_cb;
};

/** FTP data connection **/
class FTPClientData : public FTPClientBase {
    friend FTPClientBase;
public:
    /** Open FTP data connection **/
    FTPClientData(shared_ptr<FTPClientBase> ftp_cli, const char * path, Type type, TransferMode mode);
    ~FTPClientData();

    int openDataConn(shared_ptr<FTPClientBase> ftp_cli, TransferMode mode, int dir);
    int acceptConnection(shared_ptr<FTPClientBase> ftp_cli);
};

} /* namespace FEmbed */

#endif /* LIB_FE_LWIP_SRC_FTPCLIENT_H_ */
