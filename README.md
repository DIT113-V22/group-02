# Pocket Parking - Go Park Yourself! 🅿️🅿️
**What is Pocket Parking?**  

![Arduino CI](https://img.shields.io/travis/DIT113-V22/group-02?label=Arduino%20CI&style=flat-square)

Pocket Parking is an automated parking service for drivers. Our solution aims to reduce car accidents that frequently occur in parking lots. Our goal is also to introduce digitalized personal valet parking, that everyone can afford, right in your pocket!

***

**Why is Pocket Parking necessary?**  
> "20 percent of car accidents happen in parking lots" - [MotorBiscuit](https://www.motorbiscuit.com/an-alarming-number-of-car-accidents-happen-in-parking-lots/)  

Pocket Parking aims to reduce driver accidents within parking lots by taking an automated approach to the problem. Driverless parking is not only safer, it is also quite efficient and time saving.

***

**How does Pocket Parking work?**  
Pocket Parking will use the car's sensors to detect obstacles and avoid collisions, find available parking spaces and find secure paths to drive through in search of parking. It will detect presence of pedestrians and make decisions on how to keep distance to avoid harming them.

***

# Software Architecture
**Communication**
MQTT is used to establish communication between the smartcar and the mobile application. MQTT stands for "Message Queuing Telemetry Transport" and it's a protocol for devices from which they can publish/subscribe messages in order to communicate with other components within the system. MQTT is technically implemented in the smartcar and mobile application via C++ and Java code in the files below which is described further.

**smartcar.ino**
The car uses sensors such as an odometer, a gyroscope, an ultrasonic sensor and a infrared sensor. The odometer is used to get the distance traveled of the smartcar, which is a key part for some of the methods mentioned below. The gyroscope allows the vehicle to know its orientation and is used for methods that involves steering. The ultrasonic sensor is mainly used for detection of obstacles in front of the vehicle that are further away. The infrared sensors are used for the same reason as the ultrasonic sensor, but main focus here is to detect obstacles at a closer range. The smartcar.ino file uses the following methods to incorporate the business idea of the project. The handleMQTTmessages() does what the name says, it is a part of the MQTT protocol and handles messages that are sent over the broker and subscribed by the smartcar. The autoPark() and autoReverse() mehods implement the core idea of the product, which is automatic parking and retrieval. The increaseSpeed/decreaseSpeed methods handle speed of the vehicle. The turning methods handle the steering of the vehicle. The methods that handla speed and turning are sort of base methods that are logically implemented in methods such as the autoPark() and autoRetrieve(). Same thing goes with the obstacle detection methods that are a key part in the implementation the automatic parking and retrieval of the smartcar. The updateCamera() method continuously sends over data over the broker to update the camera feed in the mobile application. Finally the internal map setup serves as the fundamental section for the automatic parking and retrieval. This is the internal data representation of the current parking lot and environment of the smartcar.

**MainActivity.java**
- joyStick()

**MqttCLient.java**

***

### The Team

Armin Balesic [gusbalar@student.gu.se] 

Umar Mahmood [gusmahum@student.gu.se]

Victor Campanello [guscampvi@student.gu.se]

Aditya Khadkikar [guskhadad@student.gu.se]

Shariq Shahbaz [gusshahbsh@student.gu.se]

Ahmed Ebrahim Algabri [gusalgaah@student.gu.se]

[gusalgaah@student.gu.se]: mailto:gusalgaah@student.gu.se
[gusshahbsh@student.gu.se]: mailto:gusshahbsh@student.gu.se
[guskhadad@student.gu.se]: mailto:guskhadad@student.gu.se
[guscampvi@student.gu.se]: mailto:guscampvi@student.gu.se
[gusmahum@student.gu.se]: mailto:gusmahum@student.gu.se
[gusbalar@student.gu.se]: mailto:gusbalar@student.gu.se
