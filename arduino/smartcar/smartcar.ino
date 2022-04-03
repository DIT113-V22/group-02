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
 
const int triggerPin           = 6;  //D6
const int echoPin              = 7;  //D7
const unsigned int maxDistance = 200;
SR04 front{arduinoRuntime, triggerPin, echoPin, maxDistance};
int sensorOut = HIGH;

void setup()
{
  // Move the car with 50% of its full speed
  car.setSpeed(50);
  Serial.begin(9600);
}

void loop()
{
  const auto distance = front.getDistance();
  // The car comes to a stop if the Front UltraSonic reads a distance of 1 metre or lower.
  // The distance upper bound in the if block is slightly greater than 1 metre, however, as the car overcomes its inertia during motion first before coming to a halt.
  if (distance > 0 && distance < 150) {
    car.setSpeed(0); 
  }

#ifdef __SMCE__
  // Avoid over-using the CPU if we are running in the emulator
  delay(1);
#endif
}
