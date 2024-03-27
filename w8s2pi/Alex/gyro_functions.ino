//GYRO FUNCTIONS 
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

MPU6050 mpu;
#define OUTPUT_READABLE_YAWPITCHROLL
#define M_PI 3.141592653589793238
// MPU control/status vars
bool dmpReady = false;   // set true if DMP init was successful
uint8_t mpuIntStatus;    // holds actual interrupt status byte from MPU
uint8_t devStatus;       // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;     // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;      // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; 

Quaternion q;
VectorFloat gravity;  // [x, y, z]            gravity vector
float ypr[3];         // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector


volatile bool mpuInterrupt = false;  // indicates whether MPU interrupt pin has gone high

void dmpDataReady() {   //ISR fror the mpu interrupt routine
  mpuInterrupt = true;
}           


void setupGyro()
{
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
 // Wire.setClock(400000);  // 400kHz I2C clock. Comment this line if having compilation difficulties
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif

  mpu.initialize();

  mpu.dmpInitialize();
  

  mpu.setXGyroOffset(77);   //TODO supply gyro offsets from the IMU_Zero.ino 
  mpu.setYGyroOffset(64);
  mpu.setZGyroOffset(16);
  mpu.setZAccelOffset(2223); 

  mpu.CalibrateGyro(6);
  mpu.setDMPEnabled(true);




}
char buffer[20];

void getGyro()
{
  if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer))
  {
    #ifdef OUTPUT_READABLE_YAWPITCHROLL
    // display Euler angles in degrees
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    Serial.println(ypr[0] * 180 / M_PI);
    //dtostrf(ypr[0] * 180 / M_PI,8,2, buffer); 
    //dbprintf("hi there");  
#endif
  }
}
