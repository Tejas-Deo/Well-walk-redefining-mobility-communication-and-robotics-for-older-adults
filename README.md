# Well-walk: redefining mobility, communication, and robotics for older adults

## Product Vision
A growing number of the elderly encounter difficulty in performing daily tasks and feel isolated as they become disconnected from friends and family. These problems are exacerbated during times of pandemic when 
face-to-face visits from caretakers of all kinds are curtailed. Home robots have been proposed to address these problems but are not yet proving effective. Reviewing the available resources in the market reveals that older
adults’ options are not only limited but in many ways frustrating. Complicated user interfaces, combined with the reluctance of the elderly to embrace new technologies, make for potential solutions that never really
serve their intended targets. While technology has advanced significantly in recent years, many older adults struggle to adopt new technologies due to complex interfaces and unfamiliar user experiences. The team hypothe-
sizes that developing familiar technology, such as video conferencing and entertainment, within a simplified user interface integrated into a walker with a semi-autonomous modular kit, will provide benefits to the user 
and enable greater adoption of new technologies— especially in the years to come.

This repository demonstrates the working of the semi-autonomous system and the user interface.

<table>
  <tr>
    <td>
      <img src="/Readme_images/walker_with_cover.png" alt="Trajectories" width="100%">
      <p align="center">Walker With Cover</p>
    </td>
    <td>
      <img src="/Readme_images/walker_without_cover.png" alt="Schematic" width="100%">
      <p align="center">Walker Without Cover</p>
    </td>
  </tr>
</table>


## Devices
1. 2 Node MCUs (ESP 8266) - Deployed on the walker to control the motors and sensors (Primary Node MCU and Secondary Node MCU)
2. Raspberry Pi - Microcontroller used as a gateway to remotely control the walker
3. DC Motor - Driving Motor for the walker
4. Servo Motor - Responsible for swiveling the drive mechanism
5. Stepper Motor - A linear actuator responsible for engaging and disengaging the semi-autonomous system
6. Ultrasonic Sensors - TO alculate the distance from the nearest object and in-turn provides obstacle avoidance functionalities
7. Infrared (IR) Sensors - Transceivers used to determine the direction of movement needed to reach the charging dock, which enables semi-autonomous navigation.


<table>
  <tr>
    <td>
      <img src="/Readme_images/motors_assembly.png" alt="Trajectories" width="100%">
      <p align="center">Motors Assembly</p>
    </td>
    <td>
</table> 


## System Diagrams
The user (older adults) can control the walker via the web-application on their tablets or through Alexa Voice Commands (using alexa echo dot). The communication system diagram between the tablet and the Node MCU, and 
alexa echo dot and the Node MCU are as follows: -

<table>
  <tr>
    <td>
      <img src="/Readme_images/ui_microcontroller_communication.png" alt="Trajectories" width="100%">
      <p align="center">Communication between Web Application and Node MCUs</p>
    </td>
    <td>
      <img src="/Readme_images/voice_microcontroller_comunication.png" alt="Schematic" width="100%">
      <p align="center">Communication between echo dot and Node MCUs</p>
    </td>
  </tr>
</table>


The wiring diagram and the workload between the 2 Node MCUs is distributed as follows: -
1. Primary Node MCU - Controls the DC motor, servo motor, and the ultrasonic sensors
2. Secondary Node MCU - Controls the Stepper motor and the IR sensors

<table>
  <tr>
    <td>
      <img src="/Readme_images/WiringSchematic-1.png" alt="Trajectories" width="100%">
      <p align="center">Walker WiringDiagram</p>
    </td>
    <td>
</table> 


## Installation
1. Clone this repo using `git clone https://github.com/Tejas-Deo/Well-walk-redefining-mobility-communication-and-robotics-for-older-adults.git`
2. Install all the requirements on the Raspberry Pi using `pip install -r requirements.txt`
3. Install MQTT Mosquitto Broker
4. Install WebSockets
5. Adapt the `mosquitto.conf` file to receive commands via 2 ports; Port 1883 - MQTT, Port 9001 - WebSockets
6. Install ngrok secure tunnel
7. Create account on Amazon Developer Console to develop custom voice interaction skills to control the walker via voice commands
  

## Working of Raspberry Pi
Shoot up the Raspberry Pi and open the following terminals to get started

1. Terminal 1: To start the secure tunnel. Copy the ngrok subdomain (with "https") and use it as Alexa's endpoint in the amazon developer console to establish communication between echo dot and the raspberry pi.
```bash
$ cd /usr/local/bin
$ ./ngrok http 5000
```

2. Terminal 2: Check MQTT mosquitto broker status; make sure that the status is active
```bash
$ cd /etc/mosquitto/mosquitto.conf
$ sudo systemctl status mosquitto
```

3. Terminal 3: To check the IP address of the Raspberry Pi. Use this IP address whereever there is a variable called `mqtt_broker_address=replace_with_yours`
```bash
$ sudo ifconfig
```
Copy the inet address under wlan

4. Terminal 4: To start the Alexa Voice Commands script
```bash
$ cd /path/to/your/file
$ python alexa_rasp_code.py
```

5. Terminal 5: To start the UI commands script
```bash
$ cd /path/to/your/file
$ python new_UI_commands.py
```

6. Terminal 6: To send the charging status to the Web-Application
```bash
$ cd cd /path/to/your/file
$ python charging_status.py
```

## Working of Node MCUs
1. For pre-defined direction of Charging dock, flash the following files into Primary and Secondary Node MCU respectively
```bash
final_primary_esp.ino
final_secondary_esp.ino
```

2. To use the complete functionality of the IR sensors, flash the following files into Primary and Secondary Node MCU respectively
```bash
Primary_ESP.ino
Secodnary_ESP.ino
```


## Notes
1. To use the WebApplication for the odler adults as well as the caregivers please used the `WebApp` branch
2. `voice_recognition_code.py` script us for running custom voice recognition model directly on the Pi using the Google Assistant API (this file is not used in the final project)
