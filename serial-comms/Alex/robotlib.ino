#include <AFMotor.h>
#define SLIP_STEPS 4
#define BRAKEDELAY 500



// Motor control
#define FRONT_LEFT   4 // M4 on the driver shield
#define FRONT_RIGHT  1 // M1 on the driver shield
#define BACK_LEFT    3 // M3 on the driver shield
#define BACK_RIGHT   2 // M2 on the driver shield

AF_DCMotor motorFL(FRONT_LEFT);
AF_DCMotor motorFR(FRONT_RIGHT);
AF_DCMotor motorBL(BACK_LEFT);
AF_DCMotor motorBR(BACK_RIGHT);

void move(float speed, int direction)
{
  int speed_scaled = (speed/100.0) * 255;
  motorFL.setSpeed(speed_scaled);
  motorFR.setSpeed(speed_scaled);
  motorBL.setSpeed(speed_scaled);
  motorBR.setSpeed(speed_scaled);

  switch(direction)
    {
      case BACK:
        motorFL.run(BACKWARD);
        motorFR.run(BACKWARD);
        motorBL.run(FORWARD);
        motorBR.run(FORWARD); 
      break;
      case GO:
        motorFL.run(FORWARD);
        motorFR.run(FORWARD);
        motorBL.run(BACKWARD);
        motorBR.run(BACKWARD); 
      break;
      case CW:
        motorFL.run(FORWARD);
        motorFR.run(FORWARD);
        motorBL.run(FORWARD);
        motorBR.run(FORWARD); 
      break;
      case CCW:
        motorFL.run(BACKWARD);
        motorFR.run(BACKWARD);
        motorBL.run(BACKWARD);
        motorBR.run(BACKWARD); 
      break;
      case STOP:
      default:
        motorFL.run(RELEASE);
        motorFR.run(RELEASE);
        motorBL.run(RELEASE);
        motorBR.run(RELEASE); 
    }
}

void forward(int speed, int distance, int timeout)
{
  left_dist = 0;
  right_dist = 0;
  resumeEncoders();
  move(speed, FORWARD);
  unsigned long start_time = millis();
  unsigned long curr_dist = 0;
  while (curr_dist != distance){
    curr_dist = (left_dist > right_dist) ? left_dist : right_dist;
    if (((millis() - start_time) > timeout) || (right_dist - left_dist > SLIP_STEPS) || (left_dist - right_dist > SLIP_STEPS) || (left_dist == distance) || (right_dist == distance)){
      pauseEncoders();
      stop();
      break;
    }
  }
  delay(BRAKEDELAY);
}

void backward(int speed, int distance, int timeout)
{
  left_dist = 0;
  right_dist = 0;
  resumeEncoders();
  move(speed, BACKWARD);
  unsigned long start_time = millis();
  unsigned long curr_dist = 0;
  while (curr_dist != distance){
    curr_dist = (left_dist > right_dist) ? left_dist : right_dist;
    if (((millis() - start_time) > timeout) || (right_dist - left_dist > SLIP_STEPS) || (left_dist - right_dist > SLIP_STEPS) || (left_dist == distance) || (right_dist == distance)){
      pauseEncoders();
      stop();
      break;
    }
  }
  delay(BRAKEDELAY);
}

void ccw(int speed, int timeout)
{
  move(speed, CCW);
  delay(timeout);
  stop();
  delay(BRAKEDELAY);
}

void cw(int speed, int timeout)
{
  move(speed, CW);
  delay(timeout);
  stop();
  delay(BRAKEDELAY);
}
void stop()
{
  move(0, STOP);
}

