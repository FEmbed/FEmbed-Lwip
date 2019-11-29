/*
 * SNTP.cpp
 *
 * X-Cheng/FEmbed Project Source
 *
 *  Created on: 2019
 *  Author: Gene Kong(gyx_edu@qq.com)
 */
#include <SNTP.h>

#if SNTP_SERVER_DNS
#include "lwip/apps/sntp.h"

namespace FEmbed {

SNTP::SNTP()
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
}

SNTP::~SNTP()
{
}

void SNTP::init()
{
    sntp_init();
}

void SNTP::stop()
{
    if(sntp_enabled()){
        sntp_stop();
    }
}

bool SNTP::isEnabled()
{
    return sntp_enabled()?true:false;
}

void SNTP::setServerName(uint8_t idx, char * name)
{
    sntp_setservername(idx, (char*)name);
}

} /* namespace FEmbed */

#endif
