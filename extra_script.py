Import('env')
from os.path import join, realpath

global_env = DefaultEnvironment()
platform = env.PioPlatform()
framework = env.subst('$PIOFRAMEWORK')

CCFLAGS=["-DMQTTCLIENT_QOS2=1"]
CPPPATH=[
        realpath("src"),
        realpath("paho/MQTTClient/src"),
        ]
        
env.Prepend(CCFLAGS=CCFLAGS)
global_env.Prepend(CCFLAGS=CCFLAGS)

env.Append(CPPPATH=CPPPATH)
global_env.Append(CPPPATH=CPPPATH)

env.Replace(SRC_FILTER=["+<*>", "+<src>"])

if framework == "fembed" or "CONFIG_MQTT_USING_IBM" not in env["SDKCONFIG"]:
    env.Append(CPPPATH=[realpath("paho/MQTTPacket/src")])
    global_env.Append(CPPPATH=[realpath("paho/MQTTPacket/src")])
    env.Append(SRC_FILTER=["+<../paho/MQTTPacket/src>"])
