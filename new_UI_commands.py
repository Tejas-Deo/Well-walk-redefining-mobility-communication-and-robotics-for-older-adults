import paho.mqtt.client as mqtt
import websocket

mqtt_broker_address = "192.168.30.8"
receive_topic = "UI_receive"
send_topic = "UI"


# Callback function for when a connection is established
def on_connect_receive(client, userdata, flags, rc):
    print("Connected to receive client with result code " + str(rc))
    client.subscribe(receive_topic)


# Callback function for when a connection is established
def on_connect_send(client, userdata, flags, rc):
    print("Connected to send client with result code " + str(rc))



def publish(client, topic, msg):
    result = client.publish(topic, msg)
    status = result[0]

    if status == 0:
        print("Published the message of {} topic".format(topic))
    else:
        print('Failed to publish the message to {} topic!!!!!'.format(topic))


# Callback function for when a message is received
def on_message(client, userdata, msg):
    print("Received message: " + msg.payload.decode())


    # Publish the received message via send client
    publish(client_send, send_topic, msg.payload)
    print("Message published!!")






# Create MQTT client instances
client_receive = mqtt.Client(transport="websockets")
client_send = mqtt.Client()

# Set the callback functions for both clients
client_receive.on_connect = on_connect_receive
client_receive.on_message = on_message
client_send.on_connect = on_connect_send

# Connect to the MQTT broker via websockets on port 9001
client_receive.connect(mqtt_broker_address, 9001)

# Connect to the MQTT broker via port 1883
client_send.connect(mqtt_broker_address, 1883)

# Start the MQTT loop for both clients
client_receive.loop_start()
client_send.loop_start()

# Keep the script running
while True:
    pass

# Disconnect both clients (unreachable in this case)
client_receive.loop_stop()
client_receive.disconnect()
client_send.loop_stop()
client_send.disconnect()
