#include <MQTT.h>
#include <WiFi.h>

#include <Smartcar.h>

WiFiClient net;
MQTTClient mqtt;

ArduinoRuntime arduinoRuntime;
BrushedMotor leftMotor(arduinoRuntime, smartcarlib::pins::v2::leftMotorPins);
BrushedMotor rightMotor(arduinoRuntime, smartcarlib::pins::v2::rightMotorPins);
DifferentialControl control(leftMotor, rightMotor);

GY50 gyroscope(arduinoRuntime, 37);
const auto pulsesPerMeter = 600;
DirectionlessOdometer leftOdometer{ arduinoRuntime,smartcarlib::pins::v2::leftOdometerPin,[]() { leftOdometer.update(); },pulsesPerMeter };
DirectionlessOdometer rightOdometer{ arduinoRuntime,smartcarlib::pins::v2::rightOdometerPin,[]() { rightOdometer.update(); },pulsesPerMeter };
SmartCar car(arduinoRuntime, control, gyroscope, leftOdometer,rightOdometer);


/*--- SENSOR CONFIGURATIONS ---*/

// Front Ultrasonic Sensor
const int triggerPin = 12;  //D6
const int echoPin = 7;  //D7
const unsigned int maxDistance = 200;
SR04 front{arduinoRuntime, triggerPin, echoPin, maxDistance};


/*--- CONSTANTS ---*/
const int SPEED_INCREMENT = 5;
const int TURNING_INCREMENT = 10;
const auto oneSecond = 1000UL;



/*--- CAR INFO ---*/
int speed = 0;
int turningAngle = 0;
int heading = car.getHeading();

void setup(){
  // Move the car with 50% of its full speed
  Serial.begin(9600);
  #ifdef __SMCE__
  // ================= 1
  // mqtt.begin("aerostun.dev", 1883, WiFi);
  mqtt.begin("127.0.0.1", 1883, net); // Will connect to localhost
  #else
    mqtt.begin(net);
  #endif
    // ================= 2
    if (mqtt.connect("arduino", "public", "public")) {
      mqtt.subscribe("/smartcar/control/#", 1);
      mqtt.onMessage([](String topic, String message) {
        if (topic == "/smartcar/control/speed") {
          car.setSpeed(message.toInt());
        } else if (topic == "/smartcar/control/steering") {
          car.setAngle(message.toInt());
        } else {
          Serial.println(topic + " " + message);
        }
      });
    }
}

void loop() {

if (mqtt.connected()) {
    mqtt.loop();
    const auto currentTime = millis();
    static auto previousTransmission = 0UL;
    if (currentTime - previousTransmission >= oneSecond) {
      previousTransmission = currentTime;
      const auto distance = String(front.getDistance());
      // ================= 3
      mqtt.publish("/smartcar/ultrasound/front", distance);
    }
  }
#ifdef __SMCE__
  // Avoid over-using the CPU if we are running in the emulator
  delay(1);
#endif

  checkObstacles();
  handleInput();
  #ifdef __SMCE__
    // Avoid over-using the CPU if we are running in the emulator
    delay(1);
  #endif
}

void handleInput(){
  if(Serial.available()){
    char input = Serial.read();
    switch(input){
      case 'i':
        increaseSpeed();
        break;
      case 'k': 
        decreaseSpeed();
        break;
      case 'j': 
        turnLeft();
        break;
      case 'l': 
        turnRight();
        break;
      case 'u':
        stopCar();
        break;
      default:
        break;
    }
  }
}

void checkObstacles(){
  const auto distance = front.getDistance();
  // The car starts coming to a stop if the Front UltraSonic reads a distance of 1.5 metres or lower.
  if (distance > 0 && distance < 200 && speed>0) {
    stopCar(); 
  }
}

void increaseSpeed(){
  //sets max speed to 110
  speed = speed+SPEED_INCREMENT>=110 ? 110 : speed+SPEED_INCREMENT;
  car.setSpeed(speed);
}

void decreaseSpeed(){
  //sets max speed to 110
  speed = speed-SPEED_INCREMENT;
  car.setSpeed(speed);
}

void turnLeft(){ // turns the car 10 degrees counter-clockwise (degrees depend on TURNING_INCREMENT)
  turningAngle = turningAngle-TURNING_INCREMENT;
  car.setAngle(turningAngle);
}

void turnRight(){ // turns the car 10 degrees clockwise (degrees depend on TURNING_INCREMENT)
  turningAngle = turningAngle+TURNING_INCREMENT;
  car.setAngle(turningAngle);
}

void stopCar(){
  while(car.getSpeed() != 0){
    int speed = speed > 0 ? speed-0.1 : speed+0.1;
    car.setSpeed(speed);
  }
}
