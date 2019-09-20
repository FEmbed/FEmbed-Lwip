/*
 * MqttThread.cpp
 *
 *  Created on: 2019年9月19日
 *      Author: Gene Kong
 */

#include "MqttThread.h"
#include "TCPClient.h"

namespace FEmbed {

#define MQTT_CONFIG_RESULT_OK                                       (0)
#define MQTT_CONFIG_RESULT_CMD_ERR                                  (-1)
#define MQTT_CONFIG_RESULT_UNKNOWN                                  (-99)

#define MQTT_CONFIG_OPTION_ID_SERV                                  (0x0001)
#define MQTT_CONFIG_OPTION_ID_CONF                                  (0x0002)
#define MQTT_CONFIG_OPTION_ID_CTL                                   (0x0003)
#define MQTT_CONFIG_OPTION_ID_STA                                   (0x0004)

const static ChainConfigOptionFiledDescription mqtt_server_fields[] = {
                {
                                "<host_name>",
                                "String type. The address of the server. It could be an IP address or a domain name."
                                " The maximum size is 64 bytes",
                                ChainConfigOptionLabel_Required,
                                ChainConfigOptionType_String
                },
                {
                                "<port>",
                                "Integer type. The port of the server(Default 1883). The range is 1-65535.",
                                ChainConfigOptionLabel_Optional,
                                ChainConfigOptionType_UInteger
                },
};

const static ChainConfigOptionFiledDescription mqtt_conf_fields[] = {
                {
                                "<client_id>",
                                "String type. The client identifier string. max 63bytes",
                                ChainConfigOptionLabel_Required,
                                ChainConfigOptionType_String
                },
                {
                                "<user_name>",
                                "String type. User name of the client. It can be used for authentication. max 63bytes",
                                ChainConfigOptionLabel_Optional,
                                ChainConfigOptionType_String
                },
                {
                                "<password>",
                                "String type. Password corresponding to the user name of the client. max 63bytes",
                                ChainConfigOptionLabel_Optional,
                                ChainConfigOptionType_String
                },
                {
                                "<interval>",
                                "Integer type. Heartbeat interval(seconds).",
                                ChainConfigOptionLabel_Optional,
                                ChainConfigOptionType_UInteger
                },
                {
                                "<will_topic>",
                                "String type. Will topic string, none for ignore. max 63bytes",
                                ChainConfigOptionLabel_Optional,
                                ChainConfigOptionType_String
                },
                {
                                "<will_ctx>",
                                "String type. The Will message defines the content of the message that is"
                                " published to the will topic if the client is unexpectedly disconnected. "
                                "It can be a zero-length message. max 127bytes",
                                ChainConfigOptionLabel_Optional,
                                ChainConfigOptionType_String
                }
};

const static ChainConfigOptionDescription mqtt_options[] = {
                {
                                "SERV",
                                "Configuration MQTT Server and Port.",
                                "",     ///< Result help
                                500,
                                mqtt_server_fields,
                                ARRAY_SIZE(mqtt_server_fields),
                                MQTT_CONFIG_OPTION_ID_SERV
                },
                {
                                "CONF",
                                "Configuration MQTT Client settings.",
                                "",     ///< Result help
                                500,
                                mqtt_conf_fields,
                                ARRAY_SIZE(mqtt_conf_fields),
                                MQTT_CONFIG_OPTION_ID_SERV
                },
                {
                                "CTL",
                                "Control MQTT Client. Start, Stop or Restart.",
                                "",     ///< Result help
                                500,
                                mqtt_conf_fields,
                                ARRAY_SIZE(mqtt_conf_fields),
                                MQTT_CONFIG_OPTION_ID_CTL
                },
                {
                                "STA",
                                "Get Status of MQTT Client.",
                                "",     ///< Result help
                                500,
                                mqtt_conf_fields,
                                ARRAY_SIZE(mqtt_conf_fields),
                                MQTT_CONFIG_OPTION_ID_STA
                },

};

const static ChainConfigDescription mqtt_chain_config[] = {
               "M",
               MQTT_CONFIG_ID,
               mqtt_options,
               ARRAY_SIZE(mqtt_options)
};


MqttThread::MqttThread()
    : OSTask("MqttThread")
    , m_cli(new TCPClient())
    , m_ipstack(*m_cli)
    , m_mqtt_cli(new MQTT::Client<IPStack, Countdown, 2048>(m_ipstack))
{
    ///< use rand as default client_id;
    for(int i=0; i< 16; i++)
    {
        m_client_id[i] = 'a' + rand() % 26;
    }
    m_client_id[16] = 0;
    m_port = 1883;
    m_interval = 60;
}

MqttThread::~MqttThread()
{

}

int MqttThread::setChainConfig(
            uint32_t option_identify,
            ChainConfigOptionFiledObject* fields,
            uint32_t fields_num)
{
    int rc = MQTT_CONFIG_RESULT_OK;
    switch(option_identify)
    {
        case MQTT_CONFIG_OPTION_ID_SERV:
        {
            if(fields_num != 1 && fields_num != 2)
                rc = MQTT_CONFIG_RESULT_CMD_ERR;
            for(uint32_t i=0; i< fields_num; i++)
            {
                if(i == 0)
                {
                    if(fields[i]._is_valid == 0)
                        rc = MQTT_CONFIG_RESULT_CMD_ERR;
                    if(!this->setHost(fields[i]._enum_str))
                        rc = MQTT_CONFIG_RESULT_CMD_ERR;
                }
                else if(i == 1 && fields[i]._is_valid)
                {
                    if(!this->setPort((uint32_t) fields[i]._uint))
                        rc = MQTT_CONFIG_RESULT_CMD_ERR;
                }
            }
            break;
        }
        case MQTT_CONFIG_OPTION_ID_CONF:
        {
            if(fields_num > 6)
                rc = MQTT_CONFIG_RESULT_CMD_ERR;
            for(uint32_t i=0; i<fields_num; i++)
            {
                switch(i)
                {
                    case 0:
                    {
                        break;
                    }
                    case 1:
                    {
                        break;
                    }
                    case 2:
                    {
                        break;
                    }
                    case 3:
                    {
                        break;
                    }
                    case 4:
                    {
                        break;
                    }
                    case 5:
                    {
                        break;
                    }
                }
            }
            break;
        }
        default: rc = MQTT_CONFIG_RESULT_UNKNOWN;
    }
    return rc;
}

int MqttThread::getChainConfig(
            uint32_t option_identify,
            ChainConfigOptionFiledObject** fields,
            uint32_t *fields_num)
{
    int rc = MQTT_CONFIG_RESULT_OK;
    *fields = NULL;
    *fields_num = 0;
    switch(option_identify)
    {
        case MQTT_CONFIG_OPTION_ID_SERV:
        {

            break;
        }
        case MQTT_CONFIG_OPTION_ID_CONF:
        {

            break;
        }
        default: rc = MQTT_CONFIG_RESULT_UNKNOWN;
    }
    return rc;
}

const ChainConfigDescription *MqttThread::getConfigDescription()
{
    return mqtt_chain_config;
}

bool MqttThread::setHost(const char *host)
{
    if(strlen(host) > 63) return false;
    strcpy(m_host_name, host);
    return true;
}

bool MqttThread::setPort(uint32_t port)
{
    if(port > 0 && port < 65536)
    {
        m_port = port;
        return true;
    }
    return false;
}

bool MqttThread::setHostAndPort(const char *host, uint32_t port)
{
    if(this->setHost(host) && this->setPort(port))
        return true;
    return false;
}

bool MqttThread::setClientId(const char *client_id)
{
    if(strlen(client_id) > 63) return false;
    strcpy(m_host_name, client_id);
    return true;
}

bool MqttThread::setUserName(const char *data)
{
    if(strlen(data) > 63) return false;
    strcpy(m_user_name, data);
    return true;
}

bool MqttThread::setUserPass(const char *data)
{
    if(strlen(data) > 63) return false;
    strcpy(m_user_pass, data);
    return true;
}

bool MqttThread::setWillTopic(const char *data)
{
    if(strlen(data) > 63) return false;
    strcpy(m_will_topic, data);
    return true;
}

bool MqttThread::setWillContext(const char *data)
{
    if(strlen(data) > 63) return false;
    strcpy(m_will_ctx, data);
    return true;
}

void MqttThread::loop()
{
    for(;;)
    {
        delay(100);                 ///< Get message or Wait for 100ms to work.
    }
}

} /* namespace FEmbed */
