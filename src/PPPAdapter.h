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

#ifndef LWIP_PPPPROTOCOL_H_
#define LWIP_PPPPROTOCOL_H_

#include "osMutex.h"

#include "netif/ppp/ppp_opts.h"
#include "netif/ppp/pppapi.h"
#include "netif/ppp/ppp.h"
#include "lwip/tcpip.h"

#include <memory>
using std::shared_ptr;

#if PPP_SUPPORT
namespace FEmbed {

class PPPAdapterCallback {
 public:
    virtual ~PPPAdapterCallback() {};
    virtual uint32_t pppWrite(uint8_t *data, uint32_t len)  = 0;
};

class PPPAdapter
{
 public:
    PPPAdapter(shared_ptr<PPPAdapterCallback> cb);
    virtual ~PPPAdapter();

    enum PPPState {
        PPPState_Disconnected,      ///< Disconnected from Internet
        PPPState_Connected,         ///< Connected to Internet
        PPPState_Idle,              ///< Disconnected from Internet, Task idle, waiting for reconnect request
        PPPState_Uninit,            ///< Task started, initializing PPPoS
    };

    void updatePPPState(PPPState state) {
        OSMutexLocker locker(m_ppp_lock);
        m_state = state;
    }

    PPPState state()
    {
        OSMutexLocker locker(m_ppp_lock);
        return m_state;
    }

    void lock() { m_ppp_lock->lock(); }
    void unlock() { m_ppp_lock->unlock(); }

    netif *netIf() { return &m_ppp_netif; }

    /**
     * When send data, please call write else update from read data.
     * @param data data need process!
     * @param len data length.
     * @return success write data len.
     */
    virtual uint32_t write(uint8_t *data, uint32_t len);
    virtual void update(uint8_t *data, uint32_t len);

    /**
     * Initial PPP interface for network.
     * @return initial failed or not.
     */
    bool pppInit();

    /**
     * LCP negotiation
     * AUTH negotiation
     * IPCP negotiation
     *
     * @return negotiation ok or not?
     */
    bool pppNegotiation();

    void udpateDNSServer();

    bool pppClose();
    bool pppCloseAndFree();

 private:
    shared_ptr<OSMutex> m_ppp_lock;         ///< PPP lock for multi-thread operation.
    shared_ptr<PPPAdapterCallback> m_cb;
    PPPState m_state;
    ppp_pcb *m_ppp_pcb;
    struct netif m_ppp_netif;                      ///< Common PPP IP Interface

};

} /* namespace FEmbed */
#endif

#endif /* LWIP_PPPPROTOCOL_H_ */
