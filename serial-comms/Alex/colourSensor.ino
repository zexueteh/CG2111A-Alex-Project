#include <Arduino.h>
#include <avr/io.h>
#include <math.h>


#define PIN22 (1<<0) //S0
#define PIN23 (1<<1) //S1
#define PIN24 (1<<2) //S2
#define PIN25 (1<<3) //S3 

#define TRIG_PIN (1<<5) //digital io 27
#define ECHO_PIN (1<<6) //digital io 28

#define OUTPUT_PIN (1<<4) //pin 26

const double MOTOR_SPEED = 20;
const unsigned int DIST = 200;
const unsigned int TIMEOUT = 50;

const uint16_t RED_BLACK = 102;
const uint16_t RED_WHITE = 2270;
const uint16_t GREEN_BLACK = 114;
const uint16_t GREEN_WHITE = 2580;
const uint16_t BLUE_BLACK = 97;
const uint16_t BLUE_WHITE = 2239;

static volatile int arr[3] = {0};

#define WHITETHRESHOLD 200
#define MAXDEV 14


//assigns the rgb values into the global array.
void getRGB()
{

  //Serial.println("scanning...");

  //Serial.println("scanning red...");
  //reads red values 
  PORTA &= ~(PIN24);
  PORTA &= ~(PIN25);
  delayMicroseconds(100);
  int redValues[100] = {0};
  long redSum= 0;
  for ( int i = 0; i<100; i++)
  {
      redValues[i] = pulseIn(26,LOW);
      redSum += redValues[i];
  }
  redSum /= 100;
  //delay(2000);



  //reads green values
  //Serial.println("scanning green...");
  PORTA |= (PIN25);
  PORTA |= PIN24;
  delayMicroseconds(100);
  int greenValues[100] = {0};
  long greenSum = 0;
  for(int i = 0; i<100; i++)
  {
    greenValues[i] = pulseIn(26,LOW);
    greenSum += greenValues[i];
  }
  greenSum /= 100;
  //delay(2000);


  //reads blue values  
  //Serial.println("scanning blue...");
  PORTA &= ~(PIN24);
  PORTA |= PIN25;
  delayMicroseconds(100);
  int blueValues[100] = {0};
  long blueSum = 0;
  for(int i = 0; i<100; i++)
  {
    blueValues[i] = pulseIn(26,LOW);
    blueSum += blueValues[i];
  }
  blueSum /=100;
  //delay(2000);

  int redf =  map(redSum, RED_WHITE, RED_BLACK, 0,255);
  int greenf = map(greenSum, GREEN_WHITE, GREEN_BLACK, 0,255);
  int bluef = map(blueSum, BLUE_WHITE, BLUE_BLACK, 0, 255);

/*
  Serial.print("r: ");
  Serial.print(redf);
  Serial.print(" g: ");
  Serial.print(greenf);
  Serial.print(" b: ");
  Serial.println(bluef);
  */
  arr[0] = redf;
  arr[1] = greenf;
  arr[2] = bluef;
}


void sensorSetup() {

  DDRA |= TRIG_PIN; //DDRA sensers are for ultrasonic sensors
  DDRA &= ~(ECHO_PIN);

  DDRA |= (PIN22 | PIN23 | PIN24 | PIN24);    //DDRA registers for the color sensor  
  DDRA &= ~OUTPUT_PIN;
  PORTA |= PIN22;
  PORTA &= ~(PIN23);
    // Initialize serial communication
    //Serial.begin(9600);
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


/*
void loop() {
    // UNCOMMENT FOR COLOR SENSOR OPS 
    printColor();
    Serial.print("r: ");
    Serial.print(arr[0]);
    Serial.print(" g: ");
    Serial.print(arr[1]);
    Serial.print(" b: ");
    Serial.println(arr[2]);

    delay(1000);
    
    int dist = getDist();
    Serial.print("distance: ");
    Serial.println(dist);

}*/

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
  PORTA &= ~(TRIG_PIN);
  PORTA |= TRIG_PIN;
  delayMicroseconds(10);
  PORTA &= ~(TRIG_PIN);
  duration = pulseIn(28, HIGH);
  return (duration*0.0343)/2;

}


