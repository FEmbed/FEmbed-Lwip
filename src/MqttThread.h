/*
 * MqttThread.h
 *
 *  Created on: 2019年9月19日
 *      Author: Gene Kong
 */

#ifndef FEMBED_LWIP_SRC_MQTTTHREAD_H_
#define FEMBED_LWIP_SRC_MQTTTHREAD_H_

#include "FEmbed.h"
#include "PubSubClient.h"
#include "ChainConfig.h"

#define MQTT_CONFIG_ID                                        (0x0001)

namespace FEmbed {

class MqttThread : public OSTask, public ChainConfig
{
 public:
    MqttThread();
    virtual ~MqttThread();

    virtual int setChainConfig(
                    uint32_t option_identify,
                    ChainConfigOptionFiledObject* fields,
                    uint32_t fields_num);

    virtual int getChainConfig(
                    uint32_t option_identify,
                    ChainConfigOptionFiledObject** fields,
                    uint32_t *fields_num);

    virtual const ChainConfigDescription *getConfigDescription();

    virtual void loop();

    bool setHost(const char *host);
    bool setPort(uint32_t port);
    bool setHostAndPort(const char *host, uint32_t port);

    bool setClientId(const char *client_id);
    bool setUserName(const char *data);
    bool setUserPass(const char *data);
    bool setWillTopic(const char *data);
    bool setWillContext(const char *data);

private:
    shared_ptr<Client> m_cli;
    shared_ptr<PubSubClient> m_pub_sub_cli;

    /**
     * String is not used here. Given the possible fragmentation of memory due
     * to frequent requests for configuration, static space is used.
     */
    char m_host_name[64];        ///< Host name.
    char  m_client_id[64];       ///< Client Id, Max for 32 bytes
    char  m_user_name[64];
    char  m_user_pass[64];
    char  m_will_topic[64];
    char  m_will_ctx[128];
    uint32_t m_interval;
    uint16_t m_port;              ///< Mqtt work port.

};

} /* namespace FEmbed */

#endif /* FEMBED_LWIP_SRC_MQTTTHREAD_H_ */
