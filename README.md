# Pocket Parking - Go Park Yourself! 🅿️🅿️

&nbsp;![Android CI](https://github.com/DIT113-V22/group-02/actions/workflows/android.yml/badge.svg?branch=master)&nbsp;&nbsp;&nbsp;![Arduino CI](https://github.com/DIT113-V22/group-02/actions/workflows/arduino-build.yml/badge.svg?branch=master)

**What is Pocket Parking?**  
Pocket Parking is an automated parking service for drivers. Our solution aims to reduce car accidents that frequently occur in parking lots. Our goal is also to introduce digitalized personal valet parking, that everyone can afford, right in your pocket!

**Why is Pocket Parking necessary?**  
> "20 percent of car accidents happen in parking lots" - [MotorBiscuit](https://www.motorbiscuit.com/an-alarming-number-of-car-accidents-happen-in-parking-lots/)  

Pocket Parking aims to reduce driver accidents within parking lots by taking an automated approach to the problem. Driverless parking is not only safer, it is also quite efficient and time saving.

**How does Pocket Parking work?**  
Pocket Parking will use the car's sensors to detect obstacles and avoid collisions, find available parking spaces and find secure paths to drive through in search of parking. It will detect presence of pedestrians and make decisions on how to keep distance to avoid harming them.

# Software Architecture
**Communication**

MQTT is used to establish communication between the smartcar and the mobile application. MQTT stands for "Message Queuing Telemetry Transport" and it's a protocol for devices from which they can publish/subscribe messages in order to communicate with other components within the system. MQTT is technically implemented in the smartcar and mobile application via C++ and Java code in the files below which is described further.

**smartcar.ino**

The programming language used in smartcar is C++/Arduino. The car uses sensors such as an odometer, a gyroscope, an ultrasonic sensor and a infrared sensor. The odometer is used to get the distance traveled of the smartcar, which is a key part for some of the methods mentioned below. The gyroscope allows the vehicle to know its orientation and is used for methods that involves steering. The ultrasonic sensor is mainly used for detection of obstacles in front of the vehicle that are further away. The infrared sensors are used for the same reason as the ultrasonic sensor, but main focus here is to detect obstacles at a closer range. The smartcar.ino file uses the following methods to incorporate the business idea of the project. The handleMQTTmessages() does what the name says, it is a part of the MQTT protocol and handles messages that are sent over the broker and subscribed by the smartcar. The autoPark() and autoReverse() methods implement the core idea of the product, which is automatic parking and retrieval. The increaseSpeed/decreaseSpeed methods handle speed of the vehicle. The turning methods handle the steering of the vehicle. The methods that handla speed and turning are sort of base methods that are logically implemented in methods such as the autoPark() and autoRetrieve(). Same thing goes with the obstacle detection methods that are a key part in the implementation the automatic parking and retrieval of the smartcar. The updateCamera() method continuously sends over data over the broker to update the camera feed in the mobile application. Finally the internal map setup serves as the fundamental section for the automatic parking and retrieval. This is the internal data representation of the current parking lot and environment of the smartcar.

**MainActivity.java**

The MainActivity.java file is responsible for initializing, handling and displaying the functionality that is visible in the mobile application. The views and buttons all communicate through the IDs found in the activity_main.xml file. For instance, the camera feed is established when invoking the onCreate() method. It later then subscribes to message publications through the id in the activity_main.xml file. To the bottom right of the camera feed, there's a toggle button for switching between frontcam and birds-eye view. An external library is used for creating the joystick and is related to the setSpeed() and setAngle() methods. Additionally, connectToMQTTBroker() establishes communication between the mobile application and the broker.

**MqttClient.java**

The MqttClient.java file is mainly responsible for setting up the MQTT protocol for the Android application. In this file, there are methods that connect and disconnect the mobile application to the broker. Also methods for subscribing, unsubscribing and publishing from the mobile application is implemented in this part of the system.

# Setup Instructions


### 1. Download SMCE-gd, Mosquitto and Android Studio
&nbsp;&nbsp;&nbsp;&nbsp;Android Studio: 
https://developer.android.com/studio

&nbsp;&nbsp;&nbsp;&nbsp;SMCE_gd: 
https://github.com/ItJustWorksTM/smce-gd/releases

&nbsp;&nbsp;&nbsp;&nbsp;Mosquitto: 
https://mosquitto.org/download/


<br />

### 2. Install dependencies:

Clone project DIT113-V22/group-02, by simply clicking on the `Code` button and copy the `HTTPS` URL.

![image](https://user-images.githubusercontent.com/91271297/170710609-9947d2bd-b1b7-43fc-b6cf-a9547a00904f.png)

<br />

After copying the URL code, open Android studio upon installation. To open the application project, click on `Get from VCS`. 

<img src="https://user-images.githubusercontent.com/91271297/170711629-04547da8-2f3a-4de3-96f4-4ff81e74de50.png" width="1100">

Then paste the copied URL link to clone the project, and press `Clone`.

![image](https://user-images.githubusercontent.com/91271297/170711356-0d02667f-fd73-46f1-94b6-06096b3f16dd.png)


<br />

### 3. Register mod and vehicle

Find a Godot Game Pack (Mod) file in the GitHub repository at location group-02/mod/ppmod.pck, and copy-paste it in the mods directory of the SMCE-gd user directory (varies depending on OS), for it to register the terrain and vehicle. The location is different based on your operating system, copy-paste `ppmmod.pck` in the relevant path location from the list below.
  
**For Windows:** 
C:/Users/admin/AppData/Roaming/Godot/app_userdata/SMCE/mods  

**For Mac:** 
~/Library/Application Support/Godot/app_userdata/SMCE/mods  

**For Linux:** 
~/.local/share/godot/app_userdata/SMCE/mods/  

<br />

### 4. Running the SMCE emulator 

Start up SMCE, and click on `Start Fresh`. Next, click the `+` in the top left corner. 

Click on `+ Add New`, then select the group-02 folder. 

After that, navigate to where the cloned group-02 GitHub repository is, and in the folder `arduino/SmartCar/SmartCar.ino`, and select it as the sketch by clicking `Open`. It is the set of commands and procedures that the car requires to function, written in C/C++.  

<img src="https://user-images.githubusercontent.com/90387423/170671896-97a847fc-1870-4d04-bb14-4ff1bf451e22.png">

<br />

Next, **Press** `Compile`.
 
<img src="https://user-images.githubusercontent.com/90387423/170672110-f397fa77-f7a4-4d8d-9fae-5dc3ca2dd1cf.png">  

<br />

After successful compilation, press `Start` to spawn the smartcar. Once the car is spawned, press `Follow` to have a clear view of the car in the emulator, from the perspective of where it is heading. To get the x-ray view of the sensors beneath the car, as visualised in the second picture that follows, press `F3`.   

<img src="https://user-images.githubusercontent.com/90387423/170676764-7c1deb73-b506-4857-a12f-9fce000abf8e.png">  

<p align="center">
<img src="https://user-images.githubusercontent.com/91271297/170705787-d90bd53d-d29a-4e4c-ac40-5dc621a094d5.png" width ="100">  
</p>

![image](https://user-images.githubusercontent.com/91271297/170706023-17443e67-e1a1-4561-aaf0-3faba38a0622.png)

<br />

### 5. Setting up MQTT broker 

To run the mosquitto broker, open a compatible terminal, and type the command `mosquitto -v` to begin the connectivity protocol between the application and the smartcar. 

![image](https://user-images.githubusercontent.com/91271297/170705225-356ebacf-d459-488f-871a-9e44b1948f4c.png)

<br />

### 6. Android Application

Run the application on the Android Studio then you have a complete control of the car on the emulator 

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<img src ="https://user-images.githubusercontent.com/91271297/170490811-0b763fb7-99df-40fd-bbd2-663323f4627f.png" width="500" /> <img src ="https://user-images.githubusercontent.com/91271297/170494120-dae9e37a-4cd1-4513-92bb-0f48ac364a37.gif" width="200" height="400" />

*** 

# The Team

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
