#include <Arduino.h>
#include <avr/io.h>
#include <math.h>

//define COLOUR SENSOR pinout
#define PIN22 (1<<0) //S0  22
#define PIN24 (1<<2) //S1  23
#define PIN26 (1<<4) //S2   24
#define PIN28 (1<<6) //S3 25
#define OUTPUT_PIN (1<<7) //pin 30

//define ultrasonic pinout
#define TRIG_PIN (1<<5) //digital io 32
#define ECHO_PIN (1<<3) //digital io 34



const double MOTOR_SPEED = 20;
const unsigned int DIST = 200;
const unsigned int TIMEOUT = 50;

const uint16_t RED_BLACK = 102;
const uint16_t RED_WHITE = 2270;
const uint16_t GREEN_BLACK = 114;
const uint16_t GREEN_WHITE = 2580;
const uint16_t BLUE_BLACK = 97;
const uint16_t BLUE_WHITE = 2239;

//static volatile int arr[3] = {0};

#define WHITETHRESHOLD 200
#define MAXDEV 14


//assigns the rgb values into the global array.
void getRGB()
{

  //reads red values 
  PORTA &= ~(PIN26);
  PORTA &= ~(PIN28);
  delayMicroseconds(100);
  int redValues[100] = {0};
  long redSum= 0;
  for ( int i = 0; i<100; i++)
  {
      redValues[i] = pulseIn(30,LOW);
      redSum += redValues[i];
  }
  redSum /= 100;




  //reads green values

  PORTA |= (PIN26);
  PORTA |= PIN28;
  delayMicroseconds(100);
  int greenValues[100] = {0};
  long greenSum = 0;
  for(int i = 0; i<100; i++)
  {
    greenValues[i] = pulseIn(30,LOW);
    greenSum += greenValues[i];
  }
  greenSum /= 100;



  //reads blue values  
  PORTA &= ~(PIN26);
  PORTA |= PIN28;
  delayMicroseconds(100);
  int blueValues[100] = {0};
  long blueSum = 0;
  for(int i = 0; i<100; i++)
  {
    blueValues[i] = pulseIn(30,LOW);
    blueSum += blueValues[i];
  }
  blueSum /=100;


  int redf =  map(redSum, RED_WHITE, RED_BLACK, 0,255);
  int greenf = map(greenSum, GREEN_WHITE, GREEN_BLACK, 0,255);
  int bluef = map(blueSum, BLUE_WHITE, BLUE_BLACK, 0, 255);


  arr[0] = redf;
  arr[1] = greenf;
  arr[2] = bluef;

  Serial.print("r: ");
  Serial.print(redSum);
  Serial.print(" g: ");
  Serial.print(greenSum);
  Serial.print(" b: ");
  Serial.println(blueSum);
}


void sensorSetup() {

  DDRC |= TRIG_PIN; //DDRA sensers are for ultrasonic sensors
  DDRC &= ~(ECHO_PIN);

  DDRA |= (PIN22 | PIN24 | PIN26 | PIN28);    //DDRA registers for the color sensor  
  DDRC &= ~OUTPUT_PIN;
  PORTA |= PIN22;
  PORTA &= ~(PIN24);
    // Initialize serial communication
    
}

void closeIn()
{ 
  while (getDist()>= 7)
  {
    forward(MOTOR_SPEED, DIST, TIMEOUT);
    delay(500);
  } 
  getRGB(); 
}


float calculateSD() {
    int sum;
    float mean, SD; 
    sum = arr[0] + arr[1] + arr[2];
    mean = sum / 3;
    SD = sqrt((pow(arr[0] - mean, 2) + pow(arr[1] - mean, 2) + pow(arr[2] - mean, 2))/3);
    return (SD/mean)*100;
}

int getColour() {

  float SD; 
  getRGB();
  SD = calculateSD();
  int redIntensity = arr[0];
  int greenIntensity = arr[1];
  int blueIntensity = arr[2];

  if ((redIntensity > WHITETHRESHOLD) && (greenIntensity > WHITETHRESHOLD) && (blueIntensity > WHITETHRESHOLD) && (SD < MAXDEV)){
    return WHITE;
  }
  else if ((redIntensity > blueIntensity) && (redIntensity > greenIntensity)){
    return RED;
  }
  else if ((greenIntensity > blueIntensity) && (greenIntensity > redIntensity)){
    return GREEN;
  }  


}

void sendColour() {
	TPacket colourPacket;
	colourPacket.packetType = PACKET_TYPE_RESPONSE;
	colourPacket.command = RESP_COLOUR;
  colourPacket.params[3] = getColour();
  colourPacket.params[0] = arr[0];
  colourPacket.params[1] = arr[1];
  colourPacket.params[2] = arr[2];
  sendResponse(&colourPacket);
}

void sendDist() {
  TPacket distPacket;
  distPacket.packetType = PACKET_TYPE_RESPONSE;
  distPacket.command = RESP_DIST;

  distPacket.params[0] = getDist();
  sendResponse(&distPacket);
  
}

float getDist()
{

  long duration = 0;
  //clear trig_pin
  PORTC &= ~(TRIG_PIN);
  PORTC |= TRIG_PIN;
  delayMicroseconds(10);
  PORTC &= ~(TRIG_PIN);
  duration = pulseIn(32, HIGH);
  return (duration*0.0343)/2;

}



