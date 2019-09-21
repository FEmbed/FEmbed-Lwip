Import('env')
from os.path import join, realpath

env.Append(
    CPPPATH=[
        realpath("src"),
        realpath("paho/MQTTClient/src"),
        ])
#env.Append(CCFLAGS=["-I%s" % realpath("paho/MQTTClient/src")])
if "CONFIG_MQTT_USING_IBM" not in env["SDKCONFIG"]:
    env.Append(CPPPATH=[realpath("paho/MQTTPacket/src")])

#print(env.Dump())
