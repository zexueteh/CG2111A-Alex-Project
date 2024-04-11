#include <Arduino.h>
#include <avr/io.h>


#define PIN3 (1<<3) //S0, pin 3
#define PIN4 (1<<4) //S1
#define PIN5 (1<<5) //S2
#define PIN6 (1<<6) //S3 

#define TRIG_PIN (1<<4) //digital io 12
#define ECHO_PIN (1<<5) //digital io 13

#define OUTPUT_PIN (1<<2) //pin 2

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


int getDistance()
{
  long duration = 0;
  //clear trig_pin
  PORTB &= ~(TRIG_PIN);
  PORTB |= TRIG_PIN;
  delayMicroseconds(10);
  PORTB &= ~(TRIG_PIN);
  duration = pulseIn(13, HIGH);
  return (duration*0.0343)/2;
}

//assigns the rgb values into the global array.
void printColor()
{
  RGBTRIPLE rgbValues;
  //Serial.println("scanning...");

  //Serial.println("scanning red...");
  //reads red values 
  PORTD &= ~(PIN5);
  PORTD &= ~(PIN6);
  delayMicroseconds(100);
  int redValues[100] = {0};
  long redSum= 0;
  for ( int i = 0; i<100; i++)
  {
      redValues[i] = pulseIn(2,LOW);
      redSum += redValues[i];
  }
  redSum /= 100;
  //delay(2000);



  //reads green values
  //Serial.println("scanning green...");
  PORTD |= (PIN6);
  PORTD |= PIN5;
  delayMicroseconds(100);
  int greenValues[100] = {0};
  long greenSum = 0;
  for(int i = 0; i<100; i++)
  {
    greenValues[i] = pulseIn(2,LOW);
    greenSum += greenValues[i];
  }
  greenSum /= 100;
  //delay(2000);


  //reads blue values  
  //Serial.println("scanning blue...");
  PORTD &= ~(PIN5);
  PORTD |= PIN6;
  delayMicroseconds(100);
  int blueValues[100] = {0};
  long blueSum = 0;
  for(int i = 0; i<100; i++)
  {
    blueValues[i] = pulseIn(2,LOW);
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

  DDRB |= TRIG_PIN; //ddrb sensers are for ultrasonic sensors
  DDRB &= ~(ECHO_PIN);

  DDRD |= (PIN3 | PIN4 | PIN5 | PIN6);    //ddrd registers for the color sensor  
  DDRD &= ~OUTPUT_PIN;
  PORTD |= PIN3;
  PORTD &= ~(PIN4);
    // Initialize serial communication
    //Serial.begin(9600);
}

void closeIn()
{ 
  while (getDistance()>= 7)
  {
    forward(MOTOR_SPEED, DIST, TIMEOUT);
    delay(500);
  } 
  printColor(); 
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
    
    int dist = getDistance();
    Serial.print("distance: ");
    Serial.println(dist);

}*/
