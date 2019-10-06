Import('env')
from os.path import join, realpath

CCFLAGS=["-DMQTTCLIENT_QOS2=1"]
env.Prepend(
    CCFLAGS=CCFLAGS
)

env.AppendUnique(
    CPPPATH=[
        realpath("src"),
        realpath("paho/MQTTClient/src"),
        ])

if "CONFIG_MQTT_USING_IBM" not in env["SDKCONFIG"]:
    env.Append(CPPPATH=[realpath("paho/MQTTPacket/src")])
    env.Replace(SRC_FILTER=["+<*>", "+<../paho/MQTTPacket/src/*.c>"])


