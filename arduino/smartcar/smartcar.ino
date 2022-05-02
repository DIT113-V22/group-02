#include <Smartcar.h>
#include <stdlib.h>

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

bool obsAtFront() {
    const auto frontDist = front.getDistance();
    return (frontDist > 0 && frontDist <= 5);
}

bool obsAtFrontLong() {
    const auto frontDist = front.getDistance();
    return (frontDist > 0 && frontDist <= 40);
}

bool obsAtFrontLeft() {
    const auto frontLeftDist = frontLeft.getDistance();
    return (frontLeftDist > 0 && frontLeftDist <= 10);
}

bool obsAtFrontRight() {
    const auto frontRightDist = frontRight.getDistance();
    return (frontRightDist > 0 && frontRightDist < 40);
}

bool obsAtLeft() {
    const auto lDist = leftIR.getDistance();
    return (lDist > 0 && lDist <= 10);
}

bool obsAtRight() {
    const auto rDist = rightIR.getDistance();
    return (rDist > 0 && rDist <= 10);
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
}

void loop(){
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
        car.setSpeed(0);
        break;
      case 'p':
        autoRightPark();
        break;
      default:
        break;
    }
  }
}

void checkObstacles(){
  const auto frontDistance = front.getDistance();
  // The car starts coming to a stop if the Front UltraSonic reads a distance of 1.5 metres or lower.
  if (frontDistance > 0 && frontDistance < 1 && speed > 0) {
    speed = 0;
    car.setSpeed(speed); 
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
    // currently using these 4 timers as a way to reduce the amount of times the if-statements are true, to reduce the amount of changes to the cars turning
    int rightTimer = 1000;
    int frontRightTimer = 1000;
    int leftTimer = 1000;
    int frontLeftTimer = 1000;
    int backRightTimer = 1000;
    int backLeftTimer = 1000;
    int frontTimer = 1000;

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
    Serial.println(rightTimer);
    while (targetAngle < currentAngle){
        gyroscope.update();
        currentAngle = gyroscope.getHeading();
        if(obsAtFrontRight() && frontRightTimer > 500) { // reduce turning angle
            frontRightTimer = 0;
            turningAngle = 0;
            car.setAngle(turningAngle);
            Serial.println("front right obstacle detected");
        }
        if(obsAtFrontLeft() && frontLeftTimer > 500) { // increase turning angle, opposite of AtRight OR this should cause the car to reverse and invert/completely change the turning angle, depending on what works best
            frontLeftTimer = 0;
            turningAngle = -90;
            if (car.getSpeed() > 0){
              car.setSpeed(-15);
              Serial.println("reverve!");
            }
            car.setAngle(turningAngle);
            Serial.println("front left obstacle detected");

        }
        if(obsAtLeft() && leftTimer > 500) { // increase turning angle, opposite of AtRight OR this should cause the car to reverse and invert/completely change the turning angle, depending on what works best
            leftTimer = 0;
            turningAngle = -90;
            car.setAngle(turningAngle);
            if (car.getSpeed() > 0){
              car.setSpeed(-8);
            }
            Serial.println("left obstacle detected");
        }
        if(obsAtRight() && rightTimer > 500) { // reduce turning angle
            rightTimer = 0;
            turningAngle = 0;
            car.setAngle(turningAngle);
            if (car.getSpeed() < 0){
              car.setSpeed(8);
            }
            Serial.println("right obstacle detected");
        }
        if(obsAtFront() && frontTimer > 500){
            frontTimer = 0;
            car.setSpeed(0);
            Serial.println("obstacle at front!");
        }
        rightTimer++;
        frontRightTimer++;
        leftTimer++;
        frontLeftTimer++;
        frontTimer++;
    }
    Serial.println("turning complete, driving forward");
    car.setAngle(0);
    car.setSpeed(10);
    // here the car will have the correct angle, car will drive foward and park
}
