import time
import json
import paho.mqtt.client as mqtt

BROKER = "broker.hivemq.com"   # Public test broker
PORT = 1883
TOPIC = "farm1/soil/moisture"

client = mqtt.Client()

client.connect(BROKER, PORT, 60)

while True:
    payload = {
        "sensor": "soil_moisture",
        "value": 42,   # mock data
        "unit": "%"
    }
    client.publish(TOPIC, json.dumps(payload), qos=1)
    print(f"Published: {payload}")
    time.sleep(5)   # send every 5 seconds
