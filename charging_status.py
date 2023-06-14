import paho.mqtt.client as mqtt
import time


broker_address = '171.67.231.53'
broker_port = 9001
receive_topic_from_microncontroller = "charging_status"
send_topic_to_UI = "UI_charging"


# Callback function for when a connection is established
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker!")
    else:
        print("Failed to connect, return code: " + str(rc))

    # Subscribe to the desired topic
    client.subscribe(receive_topic_from_microncontroller)
    print("Subscribed to the topic!")


# Callback function for when a message is received
def on_message(client, userdata, msg):
    print("Received message: " + str(msg.payload.decode()))
    
    output = msg.payload.decode()

    client.publish(send_topic_to_UI, output)
    print("Message published")


# Create a new MQTT client instance
client = mqtt.Client('cg_client', transport='websockets')

# Set the username and password for the MQTT broker
client.username_pw_set(username="avatar", password="avatar")

# Assign callback functions
client.on_connect = on_connect
# client.on_message = on_message

# Connect to the MQTT broker
client.connect(broker_address, broker_port)

# Start the MQTT client loop to handle network communication and callbacks
client.loop_start()


voltage_values = [4.136, 4.088, 4.04, 3.992, 3.944, 3.848, 3.8, 3.752, 3.704, 3.656]


def publish(message):
    client.publish(send_topic_to_UI, message)
    print("Message published....")

def values():
    interval = 2 * 3800  # 2 hours in seconds
    num_values = len(voltage_values)
    delay = interval / num_values
    print("The delay is: ", delay)

    for i, value in enumerate(voltage_values):
        publish(value)
        # print(f'Published value {i+1}/{num_values}: {value}')
        time.sleep(delay)

    print('All values published')


values()

# Keep the program running to continue receiving messages
# while True:
#     pass