/*
 * SNTP.h
 *
 * X-Cheng/FEmbed Project Source
 *
 *  Created on: 2019
 *  Author: Gene Kong(gyx_edu@qq.com)
 */

#ifndef SRC_SNTP_H_
#define SRC_SNTP_H_

#include "lwipopts.h"

#if SNTP_SERVER_DNS
#include <FEmbed.h>
namespace FEmbed {

/*
 * SNTP Service for Network time
 *
 * This is simple "SNTP" wrap for the lwIP raw API. It is a minimal implementation of SNTPv4 as specified in RFC 4330.
 * For a list of some public NTP servers, see this link : http://support.ntp.org/bin/view/Servers/NTPPoolServers
 *
 */
class SNTP : public Service<SNTP>
{
 public:
     friend Service<SNTP>;
     void init();
     void stop();
     bool isEnabled();
     void setServerName(uint8_t idx, char * name);
     virtual ~SNTP();

 protected:
    SNTP();
};
} /* namespace FEmbed */

#endif

#endif /* SRC_SNTP_H_ */
