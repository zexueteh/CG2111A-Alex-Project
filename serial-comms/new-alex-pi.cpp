#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdint.h>
#include "packet.h"
#include "serial.h"
#include "serialize.h"
#include "constants.h"

#define PORT_NAME			"/dev/ttyACM0"
#define BAUD_RATE			B9600
#define NORMAL_SPEED 70 
#define FAST_SPEED 100
#define DISTANCE 100
#define TURN_NUDGE 50
#define TURN_FULL 100

int exitFlag=0;
sem_t _xmitSema;

void handleError(TResult error)
{
	switch(error)
	{
		case PACKET_BAD:
			printf("ERROR: Bad Magic Number\n");
			break;

		case PACKET_CHECKSUM_BAD:
			printf("ERROR: Bad checksum\n");
			break;

		default:
			printf("ERROR: UNKNOWN ERROR\n");
	}
}

void handleResponse(TPacket *packet)
{
	// The response code is stored in command
	switch(packet->command)
	{
		case RESP_OK:
            std::cout << "Command OK" << std::endl;
		break;

		case RESP_STATUS:
			//handleStatus(packet);
		break;

		default:
			printf("Arduino is confused\n");
	}
}

void handleErrorResponse(TPacket *packet)
{
	// The error code is returned in command
	switch(packet->command)
	{
		case RESP_BAD_PACKET:
			printf("Arduino received bad magic number\n");
		break;

		case RESP_BAD_CHECKSUM:
			printf("Arduino received bad checksum\n");
		break;

		case RESP_BAD_COMMAND:
			printf("Arduino received bad command\n");
		break;

		case RESP_BAD_RESPONSE:
			printf("Arduino received unexpected response\n");
		break;

		default:
			printf("Arduino reports a weird error\n");
	}
}

void handleMessage(TPacket *packet)
{
	printf("Message from Alex: %s\n", packet->data);
}

void handlePacket(TPacket *packet)
{
	switch(packet->packetType)
	{
		case PACKET_TYPE_COMMAND:
				// Only we send command packets, so ignore
			break;

		case PACKET_TYPE_RESPONSE:
				handleResponse(packet);
			break;

		case PACKET_TYPE_ERROR:
				handleErrorResponse(packet);
			break;

		case PACKET_TYPE_MESSAGE:
				handleMessage(packet);
			break;
	}
}

void sendPacket(TPacket *packet)
{
	char buffer[PACKET_SIZE];
	int len = serialize(buffer, packet, sizeof(TPacket));
    
	serialWrite(buffer, len);
    std::cout << "packet sent" << std::endl;
}

void *receiveThread(void *p)
{
	char buffer[PACKET_SIZE];
	int len;
	TPacket packet;
	TResult result;
	int counter=0;

	while(1)
	{
		len = serialRead(buffer);
		counter+=len;
		if(len > 0)
		{
			result = deserialize(buffer, len, &packet);

			if(result == PACKET_OK)
			{
				counter=0;
				handlePacket(&packet);
			}
			else 
				if(result != PACKET_INCOMPLETE)
				{
					printf("PACKET ERROR\n");
					handleError(result);
				}
		}
	}
}

void flushInput()
{
	char c;

	while((c = getchar()) != '\n' && c != EOF);
}


void sendCommand(char command)
{
	TPacket commandPacket;

	commandPacket.packetType = PACKET_TYPE_COMMAND;

	switch(command)
	{
		case 'w':
        case 'W':
			commandPacket.params[0] = (command == 'w')? NORMAL_SPEED: FAST_SPEED;
            commandPacket.params[1] = DISTANCE;
			commandPacket.command = COMMAND_FORWARD;
			sendPacket(&commandPacket);
			break;
		case 's':
        case 'S':
			commandPacket.params[0] = (command == 's')? NORMAL_SPEED: FAST_SPEED;
            commandPacket.params[1] = DISTANCE;
			commandPacket.command = COMMAND_REVERSE;
			sendPacket(&commandPacket);
			break;
		case 'd':
        case 'D':
			commandPacket.params[0] = NORMAL_SPEED;
            commandPacket.params[1] = (command == 's')? TURN_NUDGE: TURN_FULL;
			commandPacket.command = COMMAND_TURN_RIGHT;
			sendPacket(&commandPacket);
			break;
		case 'a':
        case 'A':
			commandPacket.params[0] = NORMAL_SPEED;
            commandPacket.params[1] = (command == 'd')? TURN_NUDGE: TURN_FULL;
			commandPacket.command = COMMAND_TURN_LEFT;
			sendPacket(&commandPacket);
			break;
        case 'q':
		case 'Q':
			exitFlag=1;
			break;
    }
}


int main()
{
	// Connect to the Arduino
	startSerial(PORT_NAME, BAUD_RATE, 8, 'N', 1, 5);

	// Sleep for two seconds
	printf("WAITING TWO SECONDS FOR ARDUINO TO REBOOT\n");
	sleep(2);
	printf("DONE\n");

	// Spawn receiver thread
	pthread_t recv;

	pthread_create(&recv, NULL, receiveThread, NULL);

	// Send a hello packet
	TPacket helloPacket;

	helloPacket.packetType = PACKET_TYPE_HELLO;
	sendPacket(&helloPacket);
	
	while(!exitFlag)
	{
        system("stty raw");
		char ch;
		ch = std::getchar();
        system("stty cooked");

		sendCommand(ch);
        //printf("\n");
        std::cout << std::endl;
        std::cout.flush();
    	}
	//system("stty cooked");
	printf("Closing connection to Arduino.\n");
	endSerial();
}
