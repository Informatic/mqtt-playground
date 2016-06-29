import paho.mqtt.client as mqtt
import time
import random

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, rc):
    print("Connected with result code "+str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("#")
    client.publish("main/status/penisy", "dupa")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print('%d %-20s %s' % (time.time(), msg.topic, msg.payload))

client = mqtt.Client("test-client-%d" % random.randint(100, 999))
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost", 1883, 60)
client.loop_forever()
