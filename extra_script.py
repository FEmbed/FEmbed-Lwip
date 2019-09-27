Import('env')
from os.path import join, realpath

CPPDEFINES=[
            ("MQTTCLIENT_QOS2", "1")
            ]

env.Append(
    CPPDEFINES=CPPDEFINES
)

env.Append(
    CPPPATH=[
        realpath("src"),
        realpath("paho/MQTTClient/src"),
        ])

if "CONFIG_MQTT_USING_IBM" not in env["SDKCONFIG"]:
    env.Append(CPPPATH=[realpath("paho/MQTTPacket/src")])
    env.Replace(SRC_FILTER=["+<*>", "+<../paho/MQTTPacket/src/*.c>"])

global_env = DefaultEnvironment()
global_env.Append(
    CPPDEFINES=CPPDEFINES
)
