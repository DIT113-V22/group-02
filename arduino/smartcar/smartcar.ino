#include <Smartcar.h>

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
const int triggerPin = 6;  //D6
const int echoPin = 7;  //D7
const unsigned int maxDistance = 200;
SR04 front{arduinoRuntime, triggerPin, echoPin, maxDistance};

//Infrared Sensors
const int leftIRPin = 1;
const int rightIRPin = 2;
const int backIRPin = 3;        
const int frontRightIRPin = 4;
const int frontLeftIRPin = 5;

typedef GP2Y0A21 infrared; //Basically a 'rename'
  infrared rightIR(arduinoRuntime, rightIRPin);
  infrared leftIR(arduinoRuntime, leftIRPin);
  infrared back(arduinoRuntime, backIRPin);
  infrared frontLeft(arduinoRuntime, frontLeftIRPin);
  infrared frontRight(arduinoRuntime, frontRightIRPin);
  
/*--- CONSTANTS ---*/
const int SPEED_INCREMENT = 5;
const int TURNING_INCREMENT = 10;

void StraightParkingFunc(){
  int angle = 0;
  const auto lDist = leftIR.getDistance();
  const auto frontDist = front.getDistance();
  const auto frontLeftDist = frontLeft.getDistance();
  const auto rDist = rightIR.getDistance();
  const auto backDist = back.getDistance();
  const auto frontRightDist = frontRight.getDistance();
  
  //Using sensors on side and back as well here to avoid collisions with obstacles in provided distances 
  if ((frontRightDist > 0 && frontRightDist < 20) && (rDist > 0 && rDist <= 20)){
    angle = angle + 10;
    car.setAngle(angle);
  }
  else if ((frontDist > 0 && frontDist <= 20) && (frontLeftDist > 0 && frontRightDist < 30) && (frontLeftDist > 0 && frontLeftDist <= 30)){
    car.setSpeed(0);
  }
  else if ((frontLeftDist > 0 && frontLeftDist < 20) && (lDist > 0 && lDist <= 20)){
    angle = angle - 10;
    car.setAngle(angle);
  }
  else if ((frontDist > 0 && frontDist <= 10) && (rDist > 0 && rDist <= 20) && (lDist > 0 && lDist <= 20) && (frontLeftDist > 0 && frontLeftDist < 20) && (frontRightDist > 0 && frontRightDist < 20)) {
    car.setSpeed(0);
    Serial.println("Car is parked!");
  }
  else {
    car.setSpeed(10);
  }
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
      default:
        break;
    }
  }
}

void checkObstacles(){
  const auto distance = front.getDistance();
  // The car starts coming to a stop if the Front UltraSonic reads a distance of 1.5 metres or lower.
  if (distance > 0 && distance < 150 && speed>0) {
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
