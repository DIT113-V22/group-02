#include <MQTT.h>
#include <WiFi.h>
#include <Smartcar.h>
#include <stdlib.h>

WiFiClient net;
MQTTClient mqtt;

ArduinoRuntime arduinoRuntime;
BrushedMotor leftMotor(arduinoRuntime, smartcarlib::pins::v2::leftMotorPins);
BrushedMotor rightMotor(arduinoRuntime, smartcarlib::pins::v2::rightMotorPins);
DifferentialControl control(leftMotor, rightMotor);

GY50 gyroscope(arduinoRuntime, 37);
const auto pulsesPerMeter = 600;
const unsigned long LEFT_PULSES_PER_METER = 600;
DirectionalOdometer leftOdometer{ arduinoRuntime,
                                 smartcarlib::pins::v2::leftOdometerPins,
                                 []() { leftOdometer.update(); },
                                 LEFT_PULSES_PER_METER };DirectionlessOdometer rightOdometer{ arduinoRuntime,smartcarlib::pins::v2::rightOdometerPin,[]() { rightOdometer.update(); },pulsesPerMeter };
SmartCar car(arduinoRuntime, control, gyroscope, leftOdometer,rightOdometer);


/*--- SENSOR CONFIGURATIONS ---*/

// Front Ultrasonic Sensor
const int triggerPin = 12;  //D6
const int echoPin = 7;  //D7
const unsigned int maxDistance = 200;
SR04 front{arduinoRuntime, triggerPin, echoPin, maxDistance};

//Infrared Sensors
const int backRightIRPin = 0;
const int leftIRPin = 1;
const int rightIRPin = 2;
const int backIRPin = 3;        
const int frontRightIRPin = 5;
const int frontLeftIRPin = 4;
const int backLeftIRPin = 6;

typedef GP2Y0A21 infrared; //Basically a 'rename'
  infrared rightIR(arduinoRuntime, rightIRPin);
  infrared leftIR(arduinoRuntime, leftIRPin);
  infrared back(arduinoRuntime, backIRPin);
  infrared frontLeft(arduinoRuntime, frontLeftIRPin);
  infrared frontRight(arduinoRuntime, frontRightIRPin);
  infrared backRight(arduinoRuntime, backRightIRPin);
  infrared backLeft(arduinoRuntime, backLeftIRPin);  
  
/*--- CONSTANTS ---*/
const int SPEED_INCREMENT = 5;
const int TURNING_INCREMENT = 10;
const int FORWARD_SPEED_LIMIT = 150;
const int BACKWARD_SPEED_LIMIT = -50;
const int MAX_STEERING_ANGLE = 60;
const auto ONE_SECOND = 1000UL;

const int BOX_WIDTH = 70;
const int BOX_HEIGHT = 35;
const int PARKING_ROWS = 5;
const int PARKING_COLS = 3;
const int ENTRANCE_R = 4;
const int ENTRANCE_C = 1;

/*--- INTERNAL MAP SETUP---*/

enum BoxType{Path, Unoccupied, Occupied, Start, NaP};

class GridBox{
  public:
    bool hasCar;
    BoxType type;
    GridBox(BoxType boxType){
      type = boxType;
    }
};

// This is the representation of the parking lot in terms of code
// The code here works for a parking lot that has infinite rows but only three columns
// i.e a parking lot of 5x3 works just as well as a parking lot of 30x3, as long as 
// the middle column remains to be the path.
GridBox parkingLot[PARKING_ROWS][PARKING_COLS] = {
    //00                01             02
    {GridBox(Occupied), GridBox(Path), GridBox(Occupied)},
    //10                11             12
    {GridBox(Unoccupied), GridBox(Path), GridBox(Occupied)},
    //20                21             22
    {GridBox(Occupied), GridBox(Path), GridBox(Occupied)},
    //30                31             32
    {GridBox(Occupied), GridBox(Path), GridBox(Occupied)},
    //40                41             42
    {GridBox(NaP),      GridBox(Start),GridBox(NaP)},
};

void park(){
    for(int i = 0; i < PARKING_ROWS; i++){
        for(int j = 0; j<PARKING_COLS; j++){
            if(parkingLot[i][j].type == Unoccupied){
                move(ENTRANCE_R, ENTRANCE_C, i, j);
                parkingLot[i][j].type = Occupied;
            }
        }
    }
}

void move(int r1, int c1, int r2, int c2){
    int parkingR = r2+1;
    int parkingC = 1;
    int currentAngle = getAngle();
    if(currentAngle > 178 && currentAngle < 182){
       // add one to take the distance of the entrace box into account
        int diffR = r1-parkingR+1;
        Serial.println(diffR);
        int distance = diffR * BOX_HEIGHT;
        move(distance);
    }
}

void move(int distance){
    leftOdometer.reset();
    while(leftOdometer.getDistance() < distance){

        car.setSpeed(10);
    }
    car.setSpeed(0);
}

bool obsAtFront() {
    const auto frontDist = front.getDistance();
    return (frontDist > 0 && frontDist <= 8);
}

bool obsAtFrontLeft() {
    const auto frontLeftDist = frontLeft.getDistance();
    return (frontLeftDist > 0 && frontLeftDist <= 8);
}

bool obsAtFrontRight() {
    const auto frontRightDist = frontRight.getDistance();
    return (frontRightDist > 0 && frontRightDist < 15);
}

bool obsAtLeft() {
    const auto lDist = leftIR.getDistance();
    return (lDist > 0 && lDist <= 10);
}

bool obsAtRight() {
    const auto rDist = rightIR.getDistance();
    return (rDist > 0 && rDist <= 40);
}

bool obsAtBackRight() {
    const auto backRightDist = backRight.getDistance();
    return (backRightDist > 0 && backRightDist < 10);
}

bool obsAtBackLeft() {
    const auto backLeftDist = backLeft.getDistance();
    return (backLeftDist > 0 && backLeftDist < 12);
}

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
      mqtt.onMessage([](String topic, String message) { handleMQTTMessage(topic, message); });
    }
}

void loop() {

if (mqtt.connected()) {
    mqtt.loop();
    const auto currentTime = millis();
    static auto previousTransmission = 0UL;
    if (currentTime - previousTransmission >= ONE_SECOND) {
      previousTransmission = currentTime;
      const auto distance = String(front.getDistance());
      // ================= 3
      //mqtt.publish("/smartcar/ultrasound/front", distance);
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


void checkObstacles(){
  const auto distance = front.getDistance();
  // The car starts coming to a stop if the Front UltraSonic reads a distance of 1.5 metres or lower.
  if (distance > 0 && distance < 200 && speed > 0) {
    stopCar();
  }
}

void stopCar(){
  while(car.getSpeed() > 0){
    int speed = speed > 0 ? speed-0.1 : speed+0.1;
    car.setSpeed(speed);
  }
}

/*--- MQTT METHODS ---*/

void handleMQTTMessage(String topic, String message){
   if (topic == "/smartcar/control/speed") {
          setSpeed(message.toFloat());
    } else if (topic == "/smartcar/control/steering") {
          setAngle(message.toFloat());
    } else {
          Serial.println(topic + " " + message);
    }
}

void setSpeed(float newSpeed){
  if(newSpeed > FORWARD_SPEED_LIMIT || newSpeed < BACKWARD_SPEED_LIMIT){
    newSpeed = newSpeed > 0 ? FORWARD_SPEED_LIMIT : BACKWARD_SPEED_LIMIT;
  }
  speed = newSpeed;
  car.setSpeed(newSpeed);
}

void setAngle(float newAngle){
  if(newAngle > MAX_STEERING_ANGLE){
    newAngle = MAX_STEERING_ANGLE;
  }
  car.setAngle(newAngle);
}

int getAngle(){
    gyroscope.update();
    return gyroscope.getHeading();
}


/*--- SERIAL METHODS ---*/

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
      case 'p':
        park();
        break;
      default:
        break;
    }
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

void autoRightPark(){ // the car is supposed to park inside a parking spot to its immediate right
    gyroscope.update();
    // currently using these timers as a way to reduce the amount of times the if-statements are true, to reduce the amount of changes to the cars turning
    int rightTimer = 500;
    int frontRightTimer = 500;
    int leftTimer = 500;
    int frontLeftTimer = 500;
    int backRightTimer = 500;
    int backLeftTimer = 500;
    int frontTimer = 500;

    // these are used so that if obsAtFrontRight() is true, the car does not stop turning while reversing
    int distanceTraveled = 0;
    int newDistanceTraveled = 0;


    int targetAngle = 0;
    int currentAngle = gyroscope.getHeading();
    if (currentAngle - 90 < 0){
        targetAngle = 360 - abs(currentAngle - 90);
    } else {
        targetAngle = currentAngle - 90;
    }
    car.setAngle(85);
    car.setSpeed(15);
    Serial.println(targetAngle);
    Serial.println(currentAngle);
    while (targetAngle <= currentAngle){
        gyroscope.update();
        newDistanceTraveled = leftOdometer.getDistance();
        currentAngle = gyroscope.getHeading();

        if(obsAtFrontRight() && frontRightTimer > 500 && newDistanceTraveled > distanceTraveled) { // reduce turning angle
            frontRightTimer = 0;
            turningAngle = 5;
            car.setAngle(turningAngle);
            car.setSpeed(10);
            Serial.println("front right obstacle detected");
        }
        if(obsAtFrontLeft() && frontLeftTimer > 500) { // reverses car and changes the turning angle to the opposite direction
            frontLeftTimer = 0;
            turningAngle = -90;
            if (car.getSpeed() > 0){
              car.setSpeed(-10);
            }
            delay(200);
            car.setAngle(turningAngle);
            Serial.println("front left obstacle detected");

        }
        if(obsAtLeft() && leftTimer > 500) { // reverses car and changes the turning angle to the opposite direction
            leftTimer = 0;
            turningAngle = -90;
            car.setAngle(turningAngle);
            if (car.getSpeed() > 0){
              car.setSpeed(-10);
            }
            Serial.println("left obstacle detected");
        }
        
        if(obsAtFront() && frontTimer > 500){ // reverses car and changes the turning angle to the opposite direction
            frontTimer = 0;
            turningAngle = -90;
            car.setAngle(turningAngle);
            if (car.getSpeed() > 0){
              car.setSpeed(-10);
            }
            Serial.println("front obstacle detected");
        }
        rightTimer++;
        frontRightTimer++;
        leftTimer++;
        frontLeftTimer++;
        frontTimer++;
        distanceTraveled = newDistanceTraveled;
    }
    car.setAngle(0);
    car.setSpeed(10);
    // here the car will have the correct angle, car will drive foward and park
}
