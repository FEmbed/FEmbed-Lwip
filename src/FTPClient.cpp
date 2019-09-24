/*
 * FTPClient.cpp
 *
 *  Created on: 2019年9月24日
 *      Author: Gene Kong
 */

#include "Arduino.h"
#include "lwip/netdb.h"
#include <malloc.h>
#include <cstdio>
#include <stdlib.h>
#include <string.h>

#include <FTPClient.h>

#ifdef  LOG_TAG
    #undef  LOG_TAG
#endif
#define LOG_TAG                             "TCPClient"

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

#define ACCEPT_TIMEOUT      (30)

namespace FEmbed {
 
/*
 * socket_wait - wait for socket to receive or flush data
 *
 * return 1 if no user callback, otherwise, return value returned by
 * user callback
 */
static int socket_wait(FTPClientBase *ctl)
{
    fd_set fd,*rfd = NULL,*wfd = NULL;
    struct timeval tv;
    int rv = 0;
    if ((ctl->dir == FTPLIB_CONTROL) || (ctl->idlecb == NULL))
	return 1;
    if (ctl->dir == FTPLIB_WRITE)
	wfd = &fd;
    else
	rfd = &fd;
    FD_ZERO(&fd);
    do
    {
        FD_SET(ctl->sock_fd,&fd);
        tv = ctl->idletime;
        rv = select(ctl->sock_fd+1, rfd, wfd, NULL, &tv);
        if (rv == -1)
        {
            rv = 0;
            strncpy(ctl->ctrl->response, strerror(errno),
                        sizeof(ctl->ctrl->response));
            break;
        }
        else if (rv > 0)
        {
            rv = 1;
            break;
        }
    }
    while ((rv = ctl->idlecb(ctl, ctl->xfered, ctl->idlearg)));
    return rv;
}

static void *memccpy(void *dest, const void *src, int c, size_t n)
{
    int i=0;
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
{
    sock_fd = -1;
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
	        end = (char *)memccpy(bp, this->cget,'\n',x);
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
	    if (!socket_wait(this))
	        return retval;
    	if ((x = lwip_read(this->sock_fd,this->cput,this->cleft)) == -1)
    	{
            log_e("read");
	        retval = -1;
	        break;
    	}
	    if (x == 0) 
	        eof = 1;
    	this->cleft -= x;
    	this->cavail += x;
    	this->cput += x;
    } while (1);
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
    for (x=0; x < len; x++)
    {
        if ((*ubp == '\n') && (lc != '\r'))
        {
            if (nb == FTPLIB_BUFSIZ)
            {
                if (!socket_wait(this))
                    return x;
                w = lwip_write(this->sock_fd, nbp, FTPLIB_BUFSIZ);
                if (w != FTPLIB_BUFSIZ)
                {
                    log_e("net_write(1) returned %d, errno = %d\n", w, errno);
                    return(-1);
                }
                nb = 0;
            }
            nbp[nb++] = '\r';
        }
        if (nb == FTPLIB_BUFSIZ)
        {
            if (!socket_wait(this))
                return x;
            w = lwip_write(this->sock_fd, nbp, FTPLIB_BUFSIZ);
            if (w != FTPLIB_BUFSIZ)
            {
                log_e("net_write(2) returned %d, errno = %d\n", w, errno);
                return(-1);
            }
            nb = 0;
        }
        nbp[nb++] = lc = *ubp++;
    }
    if (nb)
    {
	    if (!socket_wait(this))
	        return x;
	    w = lwip_write(this->sock_fd, nbp, nb);
	    if (w != nb)
	    {
            log_e("net_write(3) returned %d, errno = %d\n", w, errno);
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
        log_e("Control socket read failed");
	    return 0;
    }
    if (this->response[3] == '-')
    {
	    strncpy(match,this->response,3);
	    match[3] = ' ';
	    match[4] = '\0';
	do
	{
	    if (readLine(this->response,FTPLIB_RESPONSE_BUFSIZ) == -1)
	    {
		    log_e("Control socket read failed");
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
            if (this->buf)
                free(this->buf);
            if(this->sock_fd >= 0)
            {
                lwip_shutdown(this->sock_fd, 2);
                lwip_close(this->sock_fd);
            }
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
            if(this->sock_fd >= 0)
                lwip_close(this->sock_fd);
            break;
    }
}

int FTPClientBase::sendCmd(const char *cmd, char expresp)
{
    char buf[TMP_BUFSIZ];
    if (this->dir != FTPLIB_CONTROL)
	    return 0;

    if ((strlen(cmd) + 3) > sizeof(buf))
        return 0;
    snprintf(buf, TMP_BUFSIZ, "%s\r\n",cmd);
    if (lwip_write(this->sock_fd,buf,strlen((const char*)buf)) <= 0)
    {
	    return 0;
    }
    return readResp(expresp);
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
    int sControl;
    struct sockaddr_in sin;
    int on=1;
    char *lhost;
    char *pnum;

    memset(&sin,0,sizeof(sin));
    sin.sin_family = AF_INET;
    lhost = strdup(host);
    pnum = strchr(lhost,':');
    if (pnum == NULL)
	    pnum = "ftp";
    else
	    *pnum++ = '\0';
    if (isdigit(*pnum))
	    sin.sin_port = lwip_htons(atoi(pnum));
    else
    {
        log_e("Can't connect to %s with no-digit port number.", host);
        return false;
    }
    if ((sin.sin_addr.s_addr = inet_addr(lhost)) == INADDR_NONE)
    {
	    struct hostent *phe;
    	if ((phe = gethostbyname(lhost)) == NULL)
    	{
	        log_e("gethostbyname: %d.", h_errno);
	        free(lhost);
	        return false;
    	}
    	memcpy((char *)&sin.sin_addr, phe->h_addr, phe->h_length);
    }
    free(lhost);

    sControl = lwip_socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sControl == -1)
    {
	    log_e("socket init error.");
	    return false;
    }
    if (lwip_setsockopt(sControl,SOL_SOCKET,SO_REUSEADDR, (void *) &on, sizeof(on)) == -1)
    {
	    log_e("setsockopt");
	    lwip_close(sControl);
	    return false;
    }
    if (lwip_connect(sControl, (struct sockaddr *)&sin, sizeof(sin)) == -1)
    {
        log_e("connect to server error!");
	    lwip_close(sControl);
	    return false;
    }

    this->buf = (char *)malloc(FTPLIB_BUFSIZ);
    if (this->buf == NULL)
    {
	    log_e("calloc failed!");
	    lwip_close(sControl);
	    return false;
    }

    this->sock_fd = sControl;
    this->dir = FTPLIB_CONTROL;
    this->ctrl = NULL;
    this->data = NULL;
    this->cmode = 1;
    this->idlecb = NULL;
    this->idletime.tv_sec = this->idletime.tv_usec = 0;
    this->idlearg = NULL;
    this->xfered = 0;
    this->xfered1 = 0;
    this->cbbytes = 0;
    if (readResp('2') == 0)
    {
	    lwip_close(sControl);
	    free(this->buf);
	    this->sock_fd = -1;
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
    FILE *local = NULL;
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
	    local = fopen(localfile, ac);
	    if (local == NULL)
	    {
	        strncpy(this->response, strerror(errno),
                    sizeof(this->response));
	        return 0;
	    }
    }
    if (local == NULL)
	    local = (typ == FILE_WRITE) ? stdin : stdout;
    
    nData.reset(new FTPClientData(shared_from_this(), path, typ, mode));
    if (!nData)
    {
	    if (localfile)
	    {
	        fclose(local);
	        if ( typ == FILE_READ )
		    unlink(localfile);
	    }
        nData->ctrl = nullptr;
        this->data = nullptr;
	    return 0;
    }
    dbuf = (char *)malloc(FTPLIB_BUFSIZ);
    if (typ == FILE_WRITE)
    {
	    while ((l = fread(dbuf, 1, FTPLIB_BUFSIZ, local)) > 0)
	    {
	        if ((c = nData->write(dbuf, l)) < l)
	        {
		        std::printf("short write: passed %d, wrote %d\n", l, c);
		        rv = 0;
		        break;
	        }
	    }
    }
    else
    {
    	while ((l = nData->read(dbuf, FTPLIB_BUFSIZ)) > 0)
	    {
	        if (fwrite(dbuf, 1, l, local) == 0)
	        {
		        log_e("localfile write failed");
		        rv = 0;
		        break;
	        }
	    }
    }
    free(dbuf);
    fflush(local);
    if (localfile != NULL)
	    fclose(local);

    ///< Free nData
    nData->ctrl = nullptr;
    this->data = nullptr;
    return rv;
}

/** Log in to the server **/
int FTPClient::login(const char * user, const char * password)
{
    char tempbuf[64];
    if (((strlen(user) + 7) > sizeof(tempbuf)) ||
        ((strlen(password) + 7) > sizeof(tempbuf)))
        return 0;
    sprintf(tempbuf,"USER %s",user);
    if (!sendCmd(tempbuf,'3'))
    {
	    if (this->response[0] == '2')
	        return 1;
	    return 0;
    }
    sprintf(tempbuf,"PASS %s",password);
    return sendCmd(tempbuf,'2');
}

std::string FTPClient::getSystemType()
{
    char buf[TMP_BUFSIZ];
    int l = 0;
    char *s;
    if (!sendCmd("SYST",'2'))
	    return 0;
    s = &this->response[4];
    while ((l<TMP_BUFSIZ - 1) && (*s != ' '))
	    buf[l++] = *s++;
    buf[l] = '\0';
    return buf;
}

/** Returns current directory **/
std::string FTPClient::getDirectory()
{
    char buf[TMP_BUFSIZ];
    int l = 0;
    char *s;
    if (!sendCmd("PWD",'2'))
	    return 0;
    s = strchr(this->response, '"');
    if (s == NULL)
	    return NULL;
    s++;
    while ((l<TMP_BUFSIZ - 1) && (*s) && (*s != '"'))
	    buf[l++] = *s++;
    buf[l] = '\0';
    return buf;
}

/** Change current directory **/
int FTPClient::cd(const char * directory)
{
    char buf[TMP_BUFSIZ];
    if ((strlen(directory) + 6) > sizeof(buf))
        return 0;
    sprintf(buf,"CWD %s",directory);
    if (!sendCmd(buf,'2'))
	    return 0;
    return 1;
}

    /** Move up one level **/
int FTPClient::cdup()
{
    if (!sendCmd("CDUP",'2'))
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
    char buf[TMP_BUFSIZ];

    if ((strlen(directory) + 6) > sizeof(buf))
        return 0;
    sprintf(buf,"MKD %s",directory);
    if (!this->sendCmd(buf,'2'))
	    return 0;
    return 1;
}

/** Remove a directory **/
int FTPClient::rmdir(const char * directory)
{
    char buf[TMP_BUFSIZ];

    if ((strlen(directory) + 6) > sizeof(buf))
        return 0;
    sprintf(buf,"RMD %s",directory);
    if (!this-sendCmd(buf,'2'))
	    return 0;
    return 1;
}

/** Rename or move file **/
int FTPClient::rename(const char * oldName, const char * newName)
{
    char cmd[TMP_BUFSIZ];
    if (((strlen(oldName) + 7) > sizeof(cmd)) ||
        ((strlen(newName) + 7) > sizeof(cmd)))
        return 0;
    sprintf(cmd,"RNFR %s",oldName);
    if (!this->sendCmd(cmd,'3'))
	    return 0;
    sprintf(cmd,"RNTO %s",newName);
    if (!this->sendCmd(cmd,'2'))
	    return 0;
    return 1;
}

/** Delete a file **/
int FTPClient::unlink(const char * filename)
{
    char cmd[TMP_BUFSIZ];

    if ((strlen(filename) + 7) > sizeof(cmd))
        return 0;
    sprintf(cmd,"DELE %s",filename);
    if (!this->sendCmd(cmd,'2'))
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
    char cmd[TMP_BUFSIZ];
    int resp,rv=1;
    unsigned int sz;

    if ((strlen(path) + 7) > sizeof(cmd))
        return 0;
    sprintf(cmd, "TYPE %c", mode);
    if (!this->sendCmd(cmd, '2'))
	    return 0;
    sprintf(cmd,"SIZE %s",path);
    if (!this->sendCmd(cmd,'2'))
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
    char buf[TMP_BUFSIZ];
    int rv = 1;

    if ((strlen(path) + 7) > sizeof(buf))
        return 0;
    sprintf(buf,"MDTM %s",path);
    if (!this->sendCmd(buf,'2'))
	    rv = 0;
    else
	    strncpy(buf, &this->response[4], TMP_BUFSIZ);
    if(rv == 0)
        return NULL;
    return buf;
}

/** Run site-dependent command **/
int FTPClient::site(const char * command)
{
    char buf[TMP_BUFSIZ];
    
    if ((strlen(command) + 7) > sizeof(buf))
        return 0;
    sprintf(buf,"SITE %s",command);
    if (!this->sendCmd(buf,'2'))
	    return 0;
    return 1;
}

int FTPClientData::openDataConn(shared_ptr<FTPClientBase> ftp_ctrl, TransferMode mode, int dir)
{
    int sData;
    union {
        struct sockaddr sa;
        struct sockaddr_in in;
    } sin;
    struct linger lng = { 0, 0 };
    unsigned int l;
    int on=1;
    char *cp;
    unsigned int v[6];
    char buf[TMP_BUFSIZ];

    if (ftp_ctrl->dir != FTPLIB_CONTROL)
	    return -1;
    if ((dir != FTPLIB_READ) && (dir != FTPLIB_WRITE))
    {
	    sprintf(ftp_ctrl->response, "Invalid direction %d\n", dir);
	    return -1;
    }
    if ((mode != TEXT) && (mode != BINARY))
    {
	    sprintf(ftp_ctrl->response, "Invalid mode %c\n", mode);
	    return -1;
    }
    l = sizeof(sin);
    if (ftp_ctrl->cmode == PASSIVE)
    {
	    memset(&sin, 0, l);
	    sin.in.sin_family = AF_INET;
	    if (!ftp_ctrl->sendCmd("PASV",'2'))
	        return -1;
	    cp = strchr(ftp_ctrl->response,'(');
	    if (cp == NULL)
	        return -1;
	    cp++;
	    sscanf(cp,"%u,%u,%u,%u,%u,%u",&v[2],&v[3],&v[4],&v[5],&v[0],&v[1]);
	    sin.sa.sa_data[2] = v[2];
	    sin.sa.sa_data[3] = v[3];
	    sin.sa.sa_data[4] = v[4];
	    sin.sa.sa_data[5] = v[5];
	    sin.sa.sa_data[0] = v[0];
	    sin.sa.sa_data[1] = v[1];
    }   
    else
    {
        if (lwip_getsockname(ftp_ctrl->sock_fd, &sin.sa, &l) < 0)
        {
            log_e("getsockname failed!");
            return -1;
        }
    }
    sData = lwip_socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (sData == -1)
    {
	    log_e("socket init error!");
	    return -1;
    }
    if (lwip_setsockopt(sData,SOL_SOCKET,SO_REUSEADDR, (void *) &on,sizeof(on)) == -1)
    {
        log_e("setsockopt failed.");
	    lwip_close(sData);
	    return -1;
    }
    if (setsockopt(sData,SOL_SOCKET,SO_LINGER, (void *) &lng,sizeof(lng)) == -1)
    {
        log_e("setsockopt failed.");
	    lwip_close(sData);
	    return -1;
    }
    if (ftp_ctrl->cmode == PASSIVE)
    {
        if (lwip_connect(sData, &sin.sa, sizeof(sin.sa)) == -1)
        {
            log_e("connect failed!");
            lwip_close(sData);
            return -1;
        }
    }
    else
    {
        sin.in.sin_port = 0;
        if (lwip_bind(sData, &sin.sa, sizeof(sin)) == -1)
        {
            log_e("bind failed!");
            lwip_close(sData);
            return -1;
        }
        if (listen(sData, 1) < 0)
        {
            log_e("listen failed!");
            lwip_close(sData);
            return -1;
        }
        if (getsockname(sData, &sin.sa, &l) < 0)
            return -1;
        sprintf(buf, "PORT %d,%d,%d,%d,%d,%d",
            (unsigned char) sin.sa.sa_data[2],
            (unsigned char) sin.sa.sa_data[3],
            (unsigned char) sin.sa.sa_data[4],
            (unsigned char) sin.sa.sa_data[5],
            (unsigned char) sin.sa.sa_data[0],
            (unsigned char) sin.sa.sa_data[1]);
        if (!ftp_ctrl->sendCmd(buf,'2'))
        {
            lwip_close(sData);
            return -1;
        }
        }
        if ((mode == 'A') && ((this->buf = (char *)malloc(FTPLIB_BUFSIZ)) == NULL))
        {
            log_e("calloc failed.");
            lwip_close(sData);
            return -1;
        }
        this->sock_fd = sData;
        this->dir = dir;
        this->idletime = ftp_ctrl->idletime;
        this->idlearg = ftp_ctrl->idlearg;
        this->xfered = 0;
        this->xfered1 = 0;
        this->cbbytes = ftp_ctrl->cbbytes;
        if (this->idletime.tv_sec || this->idletime.tv_usec || this->cbbytes)
            this->idlecb = ftp_ctrl->idlecb;
        else
            this->idlecb = NULL;
        this->ctrl = ftp_ctrl;
        ftp_ctrl->data = shared_from_this();
        return 1;
}

int FTPClientData::acceptConnection(shared_ptr<FTPClientBase> ftp_ctrl)
{
    int sData;
    struct sockaddr addr;
    unsigned int l;
    int i;
    struct timeval tv;
    fd_set mask;
    int rv;

    FD_ZERO(&mask);
    FD_SET(ftp_ctrl->sock_fd, &mask);
    FD_SET(this->sock_fd, &mask);

    tv.tv_usec = 0;
    tv.tv_sec = ACCEPT_TIMEOUT;
    i = ftp_ctrl->sock_fd;
    if (i < this->sock_fd)
	i = this->sock_fd;
    i = select(i+1, &mask, NULL, NULL, &tv);
    if (i == -1)
    {
        strncpy(ftp_ctrl->response, strerror(errno),
                sizeof(ftp_ctrl->response));
        lwip_close(this->sock_fd);
        this->sock_fd = -1;
        rv = 0;
    }
    else if (i == 0)
    {
	    strcpy(ftp_ctrl->response, "timed out waiting for connection");
	    lwip_close(this->sock_fd);
	    this->sock_fd = -1;
	    rv = 0;
    }
    else
    {
        if (FD_ISSET(this->sock_fd, &mask))
        {
            l = sizeof(addr);
            sData = accept(this->sock_fd, &addr, &l);
            i = errno;
            lwip_close(this->sock_fd);
            if (sData > 0)
            {
                rv = 1;
                this->sock_fd = sData;
            }
            else
            {
                strncpy(ftp_ctrl->response, strerror(i),
                                sizeof(ftp_ctrl->response));
                this->sock_fd = -1;
                rv = 0;
            }
        }
        else if (FD_ISSET(ftp_ctrl->sock_fd, &mask))
        {
            lwip_close(this->sock_fd);
            this->sock_fd = -1;
            ftp_ctrl->readResp('2');
            rv = 0;
        }
    }
    return rv;	
}

FTPClientData::FTPClientData(shared_ptr<FTPClientBase> ftp_ctrl, const char * path, Type typ, TransferMode mode)
{
    char buf[TMP_BUFSIZ];
    int dir;
    if ((path == NULL) &&
        ((typ == FILE_WRITE) || (typ == FILE_READ)))
    {
	    sprintf(ftp_ctrl->response,
                "Missing path argument for file transfer\n");
	    return;
    }
    sprintf(buf, "TYPE %c", mode);
    if (!ftp_ctrl->sendCmd(buf, '2'))
	    return;
    switch (typ)
    {
        case DIR:
	        strcpy(buf,"NLST");
	        dir = FTPLIB_READ;
	    break;
        case DIR_VERBOSE:
	        strcpy(buf,"LIST");
	        dir = FTPLIB_READ;
	        break;
        case FILE_READ:
	        strcpy(buf,"RETR");
	        dir = FTPLIB_READ;
	        break;
        case FILE_WRITE:
	        strcpy(buf,"STOR");
	        dir = FTPLIB_WRITE;
	        break;
        default:
	        sprintf(ftp_ctrl->response, "Invalid open type %d\n", typ);
	    return;
    }
    if (path != NULL)
    {
        int i = strlen(buf);
        buf[i++] = ' ';
        if ((strlen(path) + i + 1) >= sizeof(buf))
            return;
        strcpy(&buf[i],path);
    }

    if (openDataConn(ftp_ctrl, mode, dir) == -1)
	    return;
    if (!ftp_ctrl->sendCmd(buf, '1'))
    {
	    sock_fd = -1;
	    return;
    }
    if (ftp_ctrl->cmode == PORT)
    {
	    if (!acceptConnection(ftp_ctrl))
	    {
	        sock_fd = -1;
            this->ctrl = nullptr;
	        ftp_ctrl->data = nullptr;
	    }
    }
}

FTPClientData::~FTPClientData()
{
    
}

/** Read data from a remote file or directory **/
ssize_t FTPClientData::read(void * buffer, size_t max)
{
    int i;
    if (this->dir != FTPLIB_READ)
	return 0;
    if (this->buf)
        i = this->readLine(buf, max);
    else
    {
        i = socket_wait(this);
        if (i != 1)
            return 0;
        i = lwip_read(this->sock_fd, buf, max);
    }
    if (i == -1)
	    return 0;
    this->xfered += i;
    if (this->idlecb && this->cbbytes)
    {
        this->xfered1 += i;
        if (this->xfered1 > this->cbbytes)
        {
	    if (this->idlecb(this, this->xfered, this->idlearg) == 0)
		return 0;
            this->xfered1 = 0;
        }
    }
    return i;
}

/** Write data to a remote file **/
ssize_t FTPClientData::write(void * buffer, size_t len)
{
        int i;
    if (this->dir != FTPLIB_WRITE)
	return 0;
    if (this->buf)
    	i = this->writeLine(buf, len);
    else
    {
        socket_wait(this);
        i = lwip_write(this->sock_fd, buf, len);
    }
    if (i == -1)
	return 0;
    this->xfered += i;
    if (this->idlecb && this->cbbytes)
    {
        this->xfered1 += i;
        if (this->xfered1 > this->cbbytes)
        {
            this->idlecb(this, this->xfered, this->idlearg);
            this->xfered1 = 0;
        }
    }
    return i;
}
} /* namespace FEmbed */
