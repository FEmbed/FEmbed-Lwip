#
# Component Makefile
#
COMPONENT_ADD_INCLUDEDIRS += paho/MQTTClient/src \
                             paho/MQTTPacket/src \
                             src

COMPONENT_SRCDIRS += paho/MQTTClient/src \
                     paho/MQTTPacket/src \
                     src

ifndef CONFIG_MBEDTLS
COMPONENT_OBJEXCLUDE := src/TCPClientSecure.o src/ssl_client.o
endif