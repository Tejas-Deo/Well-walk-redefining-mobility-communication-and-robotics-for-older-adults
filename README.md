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
4. Install ngrok secure tunnel
5. Create account on Amazon Developer Console to develop custom voice interaction skills to control the walker via voice commands
  

## Working
Shoot up the Raspberry Pi and open the following terminals to get started
1. Terminal 1: To start the secure tunnel
```bash
$ cd /usr/local/bin
$ ./ngrok http 5000
```
2. Terminal 2: 
