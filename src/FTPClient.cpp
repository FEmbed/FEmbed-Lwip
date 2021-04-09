/*
 * FTPClient.cpp
 *
 *  Created on: 2019年9月24日
 *      Author: Gene Kong
 */

#include "Arduino.h"
#include <malloc.h>
#include <cstdio>
#include <stdlib.h>
#include <string.h>
#include <FTPClient.h>

#ifdef  LOG_TAG
    #undef  LOG_TAG
#endif
#define LOG_TAG                             "FTPClient"

#define TMP_BUFSIZ  (256)

#define FTPLIB_CONTROL 0
#define FTPLIB_READ 1
#define FTPLIB_WRITE 2

/* connection option names */
#define FTPLIB_CONNMODE 1
#define FTPLIB_CALLBACK 2
#define FTPLIB_IDLETIME 3
#define FTPLIB_CALLBACKARG 4
#define FTPLIB_CALLBACKBYTES 5

#define ACCEPT_TIMEOUT          (30)
#define FTP_RAW_DEBUG           0
namespace FEmbed {
 
static void *memccpy(void *dest, const void *src, int c, size_t n)
{
    size_t i=0;
    const unsigned char *ip= (const uint8_t *)src;
    unsigned char *op= (uint8_t *)dest;

    while (i < n)
    {
        if ((*op++ = *ip++) == c)
            break;
        i++;
    }
    if (i == n)
        return NULL;
    return op;
}

FTPClientBase::FTPClientBase()
    : TCPClient()
{
    cput = NULL;
    cget = NULL;
    cavail = 0;
    cleft = 0;
    dir = 0;
    cmode = 0;
    idletime.tv_sec = 0;
    idletime.tv_usec = 0;
    xfered = 0;
    xfered1 = 0;
}

/*
 * read a line of text
 *
 * return -1 on error or bytecount
 */
int FTPClientBase::readLine(char *buf,int max)
{
    int x,retval = 0;
    char *end,*bp=buf;
    int eof = 0;

    if ((this->dir != FTPLIB_CONTROL) && (this->dir != FTPLIB_READ))
        return -1;
    if (max == 0)
        return 0;
    do
    {
        if (this->cavail > 0)
        {
            x = (max >= this->cavail) ? this->cavail : max-1;
            end = (char *)memccpy(bp, this->cget, '\n', x);
            if (end != NULL)
                x = end - bp;
            retval += x;
            bp += x;
            *bp = '\0';
            max -= x;
            this->cget += x;
            this->cavail -= x;
            if (end != NULL)
            {
                bp -= 2;
                if (strcmp(bp,"\r\n") == 0)
                {
                    *bp++ = '\n';
                    *bp++ = '\0';
                    --retval;
                }
                break;
            }
        }
        if (max == 1)
        {
            *buf = '\0';
            break;
        }
        if (this->cput == this->cget)
        {
            this->cput = this->cget = this->buf;
            this->cavail = 0;
            this->cleft = FTPLIB_BUFSIZ;
        }
        if (eof)
        {
            if (retval == 0)
                retval = -1;
            break;
        }
        if ((x = this->read((uint8_t *)this->cput,this->cleft)) == -1)
        {
            log_w("read failed!");
            retval = -1;
            break;
        }
        if (x == 0) 
            eof = 1;
        this->cleft -= x;
        this->cavail += x;
        this->cput += x;
        this->buf[this->cavail] = 0;
    } while (1);

#if FTP_RAW_DEBUG
    if(retval >0)
        std::printf(">>>(%d) %s", retval, buf);
#endif
    return retval;
}

/*
 * write lines of text
 *
 * return -1 on error or bytecount
 */
int FTPClientBase::writeLine(const char *buf, int len)
{
    int x, nb=0, w;
    const char *ubp = buf;
    char *nbp;
    char lc=0;

    if (this->dir != FTPLIB_WRITE)
        return -1;
    nbp = this->buf;

#if FTP_RAW_DEBUG
    std::printf("<<< %s", buf);
#endif

    for (x=0; x < len; x++)
    {
        if ((*ubp == '\n') && (lc != '\r'))
        {
            if (nb == FTPLIB_BUFSIZ)
            {
                w = this->write((uint8_t *)nbp, FTPLIB_BUFSIZ);
                if (w != FTPLIB_BUFSIZ)
                {
                    log_w("net_write(1) returned %d, errno = %d\n", w, errno);
                    return(-1);
                }
                nb = 0;
            }
            nbp[nb++] = '\r';
        }
        if (nb == FTPLIB_BUFSIZ)
        {
            w = this->write((uint8_t *)nbp, FTPLIB_BUFSIZ);
            if (w != FTPLIB_BUFSIZ)
            {
                log_w("net_write(2) returned %d, errno = %d\n", w, errno);
                return(-1);
            }
            nb = 0;
        }
        nbp[nb++] = lc = *ubp++;
    }
    if (nb)
    {
        w = this->write((uint8_t *)nbp, nb);
        if (w != nb)
        {
            log_w("net_write(3) returned %d, errno = %d\n", w, errno);
            return(-1);
        }
    }
    return len;
}

/** Returns last response **/
int FTPClientBase::readResp(char c)
{
    char match[5];
    if (readLine(this->response,FTPLIB_RESPONSE_BUFSIZ) == -1)
    {
        log_w("Control socket read failed");
        return 0;
    }
    if (this->response[3] == '-')
    {
        strncpy(match, this->response, 3);
        match[3] = ' ';
        match[4] = '\0';
        do
        {
            if (readLine(this->response,FTPLIB_RESPONSE_BUFSIZ) == -1)
            {
                log_w("Control socket read failed");
                return 0;
            }
        }
        while (strncmp(this->response,match,4));
    }
    if (this->response[0] == c)
        return 1;
    return 0;
}

FTPClientBase::~FTPClientBase()
{
    switch (this->dir)
    {
        case FTPLIB_WRITE:
            /* potential problem - if buffer flush fails, how to notify user? */
            if(this->buf != NULL)
                writeLine(NULL, 0);
        case FTPLIB_READ:
            if(this->ctrl)
            {
                if(this->ctrl->data) 
                {
                    this->ctrl->data = nullptr;
                }
                if (this->ctrl && this->ctrl->response[0] != '4' && this->ctrl->response[0] != '5')
                {
                    this->ctrl->readResp('2');
                }
                this->ctrl = nullptr;
            }
            break;
        case FTPLIB_CONTROL:
            if (this->data)
            {
                this->ctrl = nullptr;
                this->data = nullptr;
            }
            break;
    }
}

FTPClient::FTPClient()
    : FTPClientBase()
{
}

FTPClient::~FTPClient()
{
}

FTPClient::FTPClient(const char * host)
{
    this->connectHost(host);
}

bool FTPClient::connectHost(const char *host)
{
    char *lhost;
    char *pnum;
    int port;

    lhost = (char *)malloc(strlen(host) + 1);
    strcpy(lhost, host);
    pnum = strchr(lhost,':');
    if (pnum == NULL)
        pnum = (char *)"21";
    else
        *pnum++ = '\0';

    if (isdigit(*pnum))
        port = atoi(pnum);
    else
    {
        log_w("Can't connect to %s with no-digit port number.", host);
        free(lhost);
        return false;
    }

    if(this->connect(lhost, port) == -1)
    {
        log_w("connect to server %s:%d error!", lhost, port);
        free(lhost);
        return false;
    }
    free(lhost);

    this->dir = FTPLIB_CONTROL;
    this->ctrl = NULL;
    this->data = NULL;
    this->cmode = 1;
    this->idletime.tv_sec = this->idletime.tv_usec = 0;
    this->xfered = 0;
    this->xfered1 = 0;
    if (readResp('2') == 0)
    {
        this->stop();
        log_w("Connected to server %s failed with wrong response.", host);
        return false;
    }
    return true;
}

const char * FTPClient::getLastResponse()
{
    if (this->dir == FTPLIB_CONTROL)
        return this->response;
    return NULL;
}

int FTPClient::xfer(const char *localfile, const char *path, FTPClient::Type typ, FTPClient::TransferMode mode)
{
    int l,c;
    char *dbuf;
    shared_ptr<FTPClientData> nData;
    int rv=1;

    if (localfile != NULL)
    {
        char ac[4];
        memset( ac, 0, sizeof(ac) );
        if (typ == FILE_WRITE)
            ac[0] = 'r';
        else
            ac[0] = 'w';
        if (mode == BINARY)
            ac[1] = 'b';

        if(m_fs_cb) 
            m_fs_cb->open(localfile, ac);
        else
        {
            log_w("No FS implement!");
            return 0;
        }
    }
    nData.reset(new FTPClientData(shared_from_this(), path, typ, mode));
    if (!nData)
    {
        if (localfile)
        {
            m_fs_cb->close();
            if ( typ == FILE_READ )
            unlink(localfile);
        }
        nData->ctrl = nullptr;
        this->data = nullptr;
        return 0;
    }
    this->data = nData;
    dbuf = (char *)malloc(FTPLIB_BUFSIZ);
    if (typ == FILE_WRITE)
    {
        while ((l = m_fs_cb->read((uint8_t *)dbuf, FTPLIB_BUFSIZ)) > 0)
        {
            if ((c = nData->write((uint8_t *)dbuf, l)) < l)
            {
                std::printf("short write: passed %d, wrote %d\n", l, c);
                rv = 0;
                break;
            }
        }
    }
    else
    {
        while ((l = nData->read((uint8_t *)dbuf, FTPLIB_BUFSIZ)) > 0)
        {
            if (m_fs_cb->write((uint8_t *)dbuf, l) == 0)
            {
                log_w("localfile write failed");
                rv = 0;
                break;
            }
        }
    }
    free(dbuf);
    m_fs_cb->close();
    if(*nData)
    {
        nData->stop();
        this->readResp('2');
    }
        
    ///< Free nData
    nData->ctrl = nullptr;
    this->data = nullptr;
    return rv;
}

/** Log in to the server **/
int FTPClient::login(const char *user, const char *password)
{
    this->printf("USER %s\r\n",user);
    if (!readResp('3'))
    {
        if (this->response[0] == '2')
            return 1;
        return 0;
    }

    this->printf("PASS %s\r\n",password);
    return readResp('2');
}

std::string FTPClient::getSystemType()
{
    char *s, *r;
    this->printf("SYST\r\n");
    if (!readResp('2'))
        return 0;
    r = s = &this->response[4];
    
    while(s < this->response + (FTPLIB_RESPONSE_BUFSIZ - 1)&& (*s) && (*s!= ' '))
    {
        if(*s == '\\') s++;
        s++;
    }
    *s = '\0';
    return r;
}

/** Returns current directory **/
std::string FTPClient::getDirectory()
{
    char *s, *d;
    this->printf("PWD\r\n");
    if (!readResp('2'))
        return 0;
    s = strchr(this->response, '"');
    if (s == NULL)
        return NULL;
    d = ++s;
    while(s < this->response + (FTPLIB_RESPONSE_BUFSIZ - 1)&& (*s) && (*s!= '"'))
    {
        if(*s == '\\') s++;
        s++;
    }
    *s = '\0';
    return d;
}

/** Change current directory **/
int FTPClient::cd(const char * directory)
{
    this->printf("CWD %s\r\n",directory);
    if (!readResp('2'))
        return 0;
    return 1;
}

    /** Move up one level **/
int FTPClient::cdup()
{
    this->printf("CDUP\r\n");
    if (!readResp('2'))
        return 0;
    return 1;
}

/** Get terse directory listing **/
int FTPClient::getList(const char * filename, const char * path)
{
    return this->xfer(filename, path, DIR, TEXT);
}

/** Get detailed directory listing **/
int FTPClient::getFullList(const char * filename, const char * path)
{
    return this->xfer(filename, path, DIR_VERBOSE, TEXT);
}

/** Make a directory **/
int FTPClient::mkdir(const char * directory)
{
    this->printf("MKD %s\r\n",directory);
    if (!this->readResp('2'))
        return 0;
    return 1;
}

/** Remove a directory **/
int FTPClient::rmdir(const char * directory)
{
    this->printf("RMD %s\r\n",directory);
    if (!this->readResp('2'))
        return 0;
    return 1;
}

/** Rename or move file **/
int FTPClient::rename(const char * oldName, const char * newName)
{
    this->printf("RNFR %s\r\n",oldName);
    if (!this->readResp('3'))
        return 0;
    this->printf("RNTO %s\r\n",newName);
    if (!this->readResp('2'))
        return 0;
    return 1;
}

/** Delete a file **/
int FTPClient::unlink(const char * filename)
{
    this->printf("DELE %s\r\n",filename);
    if (!this->readResp('2'))
        return 0;
    return 1;
}

/** Download a file **/
int FTPClient::get(const char * local, const char * remote, FTPClient::TransferMode mode)
{
    return this->xfer(local, remote, FILE_READ, mode);
}

/** Upload a file **/
int FTPClient::put(const char * local, const char * remote, FTPClient::TransferMode mode)
{
    return this->xfer(local, remote, FILE_WRITE, mode);
}

/** Get file size **/
unsigned FTPClient::size(const char * path, FTPClient::TransferMode mode)
{
    int resp,rv=1;
    unsigned int sz;

    this->printf("TYPE %c\r\n", mode);
    if (!this->readResp('2'))
        return 0;
    this->printf("SIZE %s\r\n",path);
    if (!this->readResp('2'))
        rv = 0;
    else
    {
        if (sscanf(this->response, "%d %u", &resp, &sz) == 2)
            return sz;
        else
            rv = 0;
    }   
    return rv;
}

/** Get file modification date/time **/
std::string FTPClient::modDate(const char * path)
{
    int rv = 1;
    this->printf("MDTM %s\r\n",path);
    if (!this->readResp('2'))
        rv = 0;
    if(rv == 0)
        return NULL;
    return &this->response[4];
}

/** Run site-dependent command **/
int FTPClient::site(const char * command)
{
    this->printf("SITE %s\r\n",command);
    if (!this->readResp('2'))
        return 0;
    return 1;
}

int FTPClientData::openDataConn(shared_ptr<FTPClientBase> ftp_ctrl, TransferMode mode, int dir)
{
    union {
        uint8_t d8[4];
        uint32_t d32;
    } ip;
    uint16_t port;
    unsigned int l;
    int on=1;
    char *cp;
    unsigned int v[6];

    if (ftp_ctrl->dir != FTPLIB_CONTROL)
    {
        log_w("Invalid ctrl direction %d\n", ftp_ctrl->dir);
        return -1;
    }
    if ((dir != FTPLIB_READ) && (dir != FTPLIB_WRITE))
    {
        log_w("Invalid direction %d\n", dir);
        sprintf(ftp_ctrl->response, "Invalid direction %d\n", dir);
        return -1;
    }
    if ((mode != TEXT) && (mode != BINARY))
    {
        log_w("Invalid mode %c", mode);
        sprintf(ftp_ctrl->response, "Invalid mode %c\n", mode);
        return -1;
    }
    if (ftp_ctrl->cmode == PASSIVE)
    {
        ftp_ctrl->printf("PASV\r\n");
        if (!ftp_ctrl->readResp('2'))
        {
            log_w("PASV without response!");
            return -1;
        }
        cp = strchr(ftp_ctrl->response,'(');
        if (cp == NULL)
        {
            log_w("PASV response error:%s", ftp_ctrl->response);
            return -1;
        }
        cp++;
        sscanf(cp,"%u,%u,%u,%u,%u,%u",&v[2],&v[3],&v[4],&v[5],&v[0],&v[1]);
        ip.d8[0] = v[2];
        ip.d8[1] = v[3];
        ip.d8[2] = v[4];
        ip.d8[3] = v[5];
        port = 256 * v[0] + v[1];
    }   
    else
    {
        ip.d32 = m_sa.sin_addr.s_addr;
        port = m_sa.sin_port;
    }

    if (ftp_ctrl->cmode == PASSIVE)
    {
        this->connectV4(ip.d32, port);
    }
    else
    {
        int sData = -1;
        union {
            struct sockaddr sa;
            struct sockaddr_in in;
        } sin;
        sData = lwip_socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
        if (sData == -1)
        {
            log_w("socket init error!");
            return -1;
        }
        if (lwip_setsockopt(sData,SOL_SOCKET,SO_REUSEADDR, (void *) &on,sizeof(on)) == -1)
        {
            log_w("setsockopt failed.");
            lwip_close(sData);
            return -1;
        }

        sin.in.sin_port = 0;
        sin.in.sin_addr.s_addr = 0;
        if (lwip_bind(sData, &sin.sa, sizeof(sin)) == -1)
        {
            log_w("bind failed!");
            lwip_close(sData);
            return -1;
        }
        if (lwip_listen(sData, 1) < 0)
        {
            log_w("listen failed!");
            lwip_close(sData);
            return -1;
        }
        if (lwip_getsockname(sData, &sin.sa, (socklen_t *)&l) < 0)
            return -1;
        this->printf("PORT %d,%d,%d,%d,%d,%d\r\n",
            (unsigned char) sin.sa.sa_data[2],
            (unsigned char) sin.sa.sa_data[3],
            (unsigned char) sin.sa.sa_data[4],
            (unsigned char) sin.sa.sa_data[5],
            (unsigned char) sin.sa.sa_data[0],
            (unsigned char) sin.sa.sa_data[1]);
        if (!ftp_ctrl->readResp('2'))
        {
            lwip_close(sData);
            return -1;
        }

        this->m_socket_fd = sData;
    }

    this->dir = dir;
    this->idletime = ftp_ctrl->idletime;
    this->xfered = 0;
    this->xfered1 = 0;
    this->ctrl = ftp_ctrl;
    return 1;
}

int FTPClientData::acceptConnection(shared_ptr<FTPClientBase> ftp_ctrl)
{
    ///< Port Mode Supported later!
    (void) ftp_ctrl;
    log_w("current version not support PORT mode.");
    return -1;
}

FTPClientData::FTPClientData(shared_ptr<FTPClientBase> ftp_ctrl,
                                const char * path, Type typ, TransferMode mode)
    : FTPClientBase()
{
    if ((path == NULL) &&
        ((typ == FILE_WRITE) || (typ == FILE_READ)))
    {
        log_w("Missing path argument for file transfer");
        sprintf(ftp_ctrl->response,
                "Missing path argument for file transfer\n");
        return;
    }
    ftp_ctrl->printf("TYPE %c\r\n", mode);
    if (!ftp_ctrl->readResp('2'))
    {
        log_w("TYPE %c without response.", mode);
        return;
    }
    if (openDataConn(ftp_ctrl, mode, typ==FILE_WRITE?FTPLIB_WRITE:FTPLIB_READ) == -1)
    {
        log_w("openDataConn failed.");
        return;
    }

    switch (typ)
    {
        case DIR:
            ftp_ctrl->printf("NLST ");
        break;
        case DIR_VERBOSE:
            ftp_ctrl->printf("LIST ");
            break;
        case FILE_READ:
            ftp_ctrl->printf("RETR ");
            break;
        case FILE_WRITE:
            ftp_ctrl->printf("STOR ");
            break;
        default:
            std::sprintf(ftp_ctrl->response, "Invalid open type %d\n", typ);
        return;
    }
    
    ftp_ctrl->printf("%s\r\n", path);
    if (!ftp_ctrl->readResp('1'))
    {
        this->stop();
        return;
    }
    if (ftp_ctrl->cmode == PORT)
    {
        if (!acceptConnection(ftp_ctrl))
        {
            this->stop();
            this->ctrl = nullptr;
            ftp_ctrl->data = nullptr;
        }
    }
}

FTPClientData::~FTPClientData()
{
}

} /* namespace FEmbed */
