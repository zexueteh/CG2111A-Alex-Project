#include <serialize.h>
#include <stdarg.h>
#include "packet.h"
#include "constants.h"


/*
   Alex's configuration constants
 */

// Number of ticks per revolution from the
// wheel encoder.

#define COUNTS_PER_REV      16

// Wheel circumference in cm.
// We will use this to calculate forward/backward distance traveled
// by taking revs * WHEEL_CIRC

#define WHEEL_CIRC          21.362812

#define MOTOR_TIMEOUT 2000







volatile uint8_t motor_status = STOPPED; 
volatile uint8_t left_dist = 0;
volatile uint8_t right_dist = 0;
static volatile int arr[3] = {0};

/*

   Alex Communication Routines.

 */


TResult readPacket(TPacket *packet)
{
	// Reads in data from the serial port and
	// deserializes it.Returns deserialized
	// data in "packet".

	char buffer[PACKET_SIZE];
	int len;

	len = readSerial(buffer);

	if (len == 0)
		return PACKET_INCOMPLETE;
	else
		return deserialize(buffer, len, packet);

}

// void sendStatus()
// {
// 	// Implement code to send back a packet containing key
// 	// information like leftTicks, rightTicks, leftRevs, rightRevs
// 	// forwardDist and reverseDist
// 	// Use the params array to store this information, and set the
// 	// packetType and command files accordingly, then use sendResponse
// 	// to send out the packet. See sendMessage on how to use sendResponse.
// 	//
// 	TPacket statusPacket;
// 	statusPacket.packetType = PACKET_TYPE_RESPONSE;
// 	statusPacket.command = RESP_STATUS;
// 	// statusPacket.params[0] = leftForwardTicks;
// 	// statusPacket.params[1] = rightForwardTicks;
// 	// statusPacket.params[2] = leftReverseTicks;
// 	// statusPacket.params[3] = rightReverseTicks;
// 	// statusPacket.params[4] = leftForwardTicksTurns;
// 	// statusPacket.params[5] = rightForwardTicksTurns;
// 	// statusPacket.params[6] = leftReverseTicksTurns;
// 	// statusPacket.params[7] = rightReverseTicksTurns;
// 	// statusPacket.params[8] = forwardDist;
// 	// statusPacket.params[9] = reverseDist;
// 	sendResponse(&statusPacket);
// }

void sendMessage(const char *message)
{
	// Sends text messages back to the Pi. Useful
	// for debugging.

	TPacket messagePacket;
	messagePacket.packetType = PACKET_TYPE_MESSAGE;
	strncpy(messagePacket.data, message, MAX_STR_LEN);
	sendResponse(&messagePacket);
}

void dbprintf(char *format, ...) {
 va_list args;
 char buffer[128];
 va_start(args, format);
 vsprintf(buffer, format, args);
 sendMessage(buffer);
}

void sendBadPacket()
{
	// Tell the Pi that it sent us a packet with a bad
	// magic number.

	TPacket badPacket;
	badPacket.packetType = PACKET_TYPE_ERROR;
	badPacket.command = RESP_BAD_PACKET;
	sendResponse(&badPacket);

}

void sendBadChecksum()
{
	// Tell the Pi that it sent us a packet with a bad
	// checksum.

	TPacket badChecksum;
	badChecksum.packetType = PACKET_TYPE_ERROR;
	badChecksum.command = RESP_BAD_CHECKSUM;
	sendResponse(&badChecksum);
}

void sendBadCommand()
{
	// Tell the Pi that we don't understand its
	// command sent to us.

	TPacket badCommand;
	badCommand.packetType = PACKET_TYPE_ERROR;
	badCommand.command = RESP_BAD_COMMAND;
	sendResponse(&badCommand);
}

void sendBadResponse()
{
	TPacket badResponse;
	badResponse.packetType = PACKET_TYPE_ERROR;
	badResponse.command = RESP_BAD_RESPONSE;
	sendResponse(&badResponse);
}

void sendOK()
{
	TPacket okPacket;
	okPacket.packetType = PACKET_TYPE_RESPONSE;
	okPacket.command = RESP_OK;
	sendResponse(&okPacket);
}

void sendResponse(TPacket *packet)
{
	// Takes a packet, serializes it then sends it out
	// over the serial port.
	char buffer[PACKET_SIZE];
	int len;

	len = serialize(buffer, packet, sizeof(TPacket));
	writeSerial(buffer, len);
}


/*
   Setup and start codes for external interrupts and
   pullup resistors.

 */
// Enable pull up resistors on pins 18 and 19
void enablePullups()
{
	// Use bare-metal to enable the pull-up resistors on pins
	// 19 and 18. These are pins PD2 and PD3 respectively.
	// We set bits 2 and 3 in DDRD to 0 to make them inputs.
	DDRD &= 0b11110011;
	PORTD |= 0b00001100;
}

// Functions to be called by INT2 and INT3 ISRs.
ISR(INT2_vect)
{
  left_dist++;
}

ISR(INT3_vect)
{
  right_dist++;
}

void startEncoders()
{
  DDRD &= 0b11110011;
  PORTD |= 0b00001100;
  EIMSK = 0b00000000;
  EICRA = 0b10100000;
}
void pauseEncoders(){
  EIMSK &= 0b11110011;
}
void resumeEncoders(){
  EIMSK |= 0b00001100;
}



/*
   Setup and start codes for serial communications

 */
// Set up the serial connection. For now we are using
// Arduino Wiring, you will replace this later
// with bare-metal code.
void setupSerial()
{
	// To replace later with bare-metal.
	Serial.begin(9600);
	// Change Serial to Serial2/Serial3/Serial4 in later labs when using the other UARTs
}

// Start the serial connection. For now we are using
// Arduino wiring and this function is empty. We will
// replace this later with bare-metal code.

void startSerial()
{
	// Empty for now. To be replaced with bare-metal code
	// later on.

}

// Read the serial port. Returns the read character in
// ch if available. Also returns TRUE if ch is valid.
// This will be replaced later with bare-metal code.

int readSerial(char *buffer)
{

	int count = 0;

	// Change Serial to Serial2/Serial3/Serial4 in later labs when using other UARTs

	while (Serial.available())
		buffer[count++] = Serial.read();

	return count;
}

// Write to the serial port. Replaced later with
// bare-metal code

void writeSerial(const char *buffer, int len)
{
	Serial.write(buffer, len);
	// Change Serial to Serial2/Serial3/Serial4 in later labs when using other UARTs
}




#define CW_TIMEOUT 1500
#define CCW_TIMEOUT 1500

void handleCommand(TPacket *command)
{
	switch (command->command)
	{
		// For movement commands, param[0] = distance, param[1] = speed, param[2] = timeout.
		case COMMAND_FORWARD:


			forward((double) command->params[0], 200, (float) command->params[1]);

      sendOK();
			break;

		case COMMAND_REVERSE:
			backward((double) command->params[0], 200, (float) command->params[1]);
      sendOK();

      break;

		case COMMAND_TURN_LEFT:
			
			ccw((double) command->params[0], (float)command->params[1]);
			sendOK();
			break;

		case COMMAND_TURN_RIGHT:
			cw((double) command->params[0], (float)command->params[1]);
			sendOK();
			break;
		case COMMAND_STOP:
			stop();
      sendOK();
			break;
		case COMMAND_GET_COLOUR:
			sendOK();
			sendColour();
			break;
		case COMMAND_GET_DIST:
			sendOK();
			sendDist();
			break;

		default:
			sendBadCommand();
	}
}

void waitForHello()
{
	int exit = 0;

	while (!exit)
	{
		TPacket hello;
		TResult result;

		do
		{
			result = readPacket(&hello);
		} while (result == PACKET_INCOMPLETE);

		if (result == PACKET_OK)
		{
			if (hello.packetType == PACKET_TYPE_HELLO)
			{


				sendOK();
				exit = 1;
			}
			else
				sendBadResponse();
		}
		else if (result == PACKET_BAD)
		{
			sendBadPacket();
		}
		else if (result == PACKET_CHECKSUM_BAD)
			sendBadChecksum();
	} // !exit
}

void setup() {
	// put your setup code here, to run once:

	cli();
  startEncoders();
	setupSerial();
  sensorSetup();
	startSerial();
	enablePullups();
	sei();
  Serial.begin(9600);

}

void handlePacket(TPacket *packet)
{
	switch (packet->packetType)
	{
		case PACKET_TYPE_COMMAND:
			handleCommand(packet);
			break;

		case PACKET_TYPE_RESPONSE:
			break;

		case PACKET_TYPE_ERROR:
			break;

		case PACKET_TYPE_MESSAGE:
			break;

		case PACKET_TYPE_HELLO:
			break;
	}
}

void loop() {
  
	// Uncomment the code below for Step 2 of Activity 3 in Week 8 Studio 2

	//forward(0, 100);

	// Uncomment the code below for Week 9 Studio 2


	// put your main code here, to run repeatedly:
  TPacket recvPacket;
	TResult result = readPacket(&recvPacket);

	if (result == PACKET_OK)
		handlePacket(&recvPacket);
	else if (result == PACKET_BAD)
	{
		sendBadPacket();
	}
	else if (result == PACKET_CHECKSUM_BAD)
	{
		sendBadChecksum();
	}
  /*getRGB();
  Serial.print("r: ");
  Serial.print();
  Serial.print(" g: ");
  Serial.print(greenSum);
  Serial.print(" b: ");
  Serial.println(blueSum);
  
  //Serial.println(getColour());
*/
}
