
// Routines to create a TLS client
#include "make_tls_client.h"

// Network packet types
#include "netconstants.h"

// Packet types, error codes, etc.
#include "constants.h"

#define STRAIGHT_SPEED 35
#define TURN_SPEED 70
#define TIMEOUT_SHORT 100
#define TIMEOUT_LONG 300
#define TURN_NUDGE 100
#define TURN_FULL 300

// Tells us that the network is running.
static volatile int networkActive=0;

void handleError(const char *buffer)
{
	switch(buffer[1])
	{
		case RESP_OK:
			printf("Command / Status OK\n");
			break;

		case RESP_BAD_PACKET:
			printf("BAD MAGIC NUMBER FROM ARDUINO\n");
			break;

		case RESP_BAD_CHECKSUM:
			printf("BAD CHECKSUM FROM ARDUINO\n");
			break;

		case RESP_BAD_COMMAND:
			printf("PI SENT BAD COMMAND TO ARDUINO\n");
			break;

		case RESP_BAD_RESPONSE:
			printf("PI GOT BAD RESPONSE FROM ARDUINO\n");
			break;

		default:
			printf("PI IS CONFUSED!\n");
	}
}

void handleStatus(const char *buffer)
{
	int32_t data[16];
	memcpy(data, &buffer[1], sizeof(data));

	printf("\n ------- ALEX STATUS REPORT ------- \n\n");
	printf("Left Forward Ticks:\t\t%d\n", data[0]);
	printf("Right Forward Ticks:\t\t%d\n", data[1]);
	printf("Left Reverse Ticks:\t\t%d\n", data[2]);
	printf("Right Reverse Ticks:\t\t%d\n", data[3]);
	printf("Left Forward Ticks Turns:\t%d\n", data[4]);
	printf("Right Forward Ticks Turns:\t%d\n", data[5]);
	printf("Left Reverse Ticks Turns:\t%d\n", data[6]);
	printf("Right Reverse Ticks Turns:\t%d\n", data[7]);
	printf("Forward Distance:\t\t%d\n", data[8]);
	printf("Reverse Distance:\t\t%d\n", data[9]);
	printf("\n---------------------------------------\n\n");
}

void handleMessage(const char *buffer)
{
	printf("MESSAGE FROM ALEX: %s\n", &buffer[1]);
}

void handleCommand(const char *buffer)
{
	// We don't do anything because we issue commands
	// but we don't get them. Put this here
	// for future expansion
}

void handleColour(const char *buffer)
{
	int32_t data[4];
	memcpy(data, &buffer[1], sizeof(data)); // buffer[1] might change

	printf("Red Intensity:\t\t%d\n", data[0]);
	printf("Green Intensity:\t\t%d\n", data[1]);
	printf("Blue Intensity:\t\t%d\n", data[2]);
	printf("Colour Detected:\t\t%d\n", data[3]);
}

void handleDist(const char *buffer)
{
	int32_t data[1];
	memcpy(data, &buffer[1], sizeof(data));

	printf("Distance:\t\t%d\n", data[0]);
}

void handleNetwork(const char *buffer, int len)
{
	// The first byte is the packet type
	int type = buffer[0];

	switch(type)
	{
		case NET_ERROR_PACKET:
			handleError(buffer);
			break;

		case NET_STATUS_PACKET:
			handleStatus(buffer);
			break;

		case NET_MESSAGE_PACKET:
			handleMessage(buffer);
			break;

		case NET_COMMAND_PACKET:
			handleCommand(buffer);
			break;
		
		case NET_COLOUR_PACKET:
			handleColour(buffer);
			break;
		
		case NET_DIST_PACKET:
			handleDist(buffer);
			break;
	}
}

void sendData(void *conn, const char *buffer, int len)
{
	int c;
	printf("\nSENDING %d BYTES DATA\n\n", len);
	if(networkActive)
	{
		/* TODO: Insert SSL write here to write buffer to network */

		sslWrite(conn, buffer, len);
		/* END TODO */	
		networkActive = (c > 0);
	}
}

void *readerThread(void *conn)
{
	char buffer[128];
	int len;

	while(networkActive)
	{
		/* TODO: Insert SSL read here into buffer */
		len = sslRead(conn, buffer, 128);
		printf("read %d bytes from server.\n", len);

		/* END TODO */

		networkActive = (len > 0);

		if(networkActive)
			handleNetwork(buffer, len);
	}

	printf("Exiting network listener thread\n");

	/* TODO: Stop the client loop and call EXIT_THREAD */
	stopClient();
	EXIT_THREAD(conn);	
	/* END TODO */

	return NULL;
}

void flushInput()
{
	char c;

	while((c = getchar()) != '\n' && c != EOF);
}

void *writerThread(void *conn)
{
	int quit=0;

	while(!quit)
	{
		char ch;
		printf("Command (w=forward, s=reverse, a=turn left, d=turn right, q=exit)\n");
		scanf("%c", &ch);

		// Purge extraneous characters from input stream
		flushInput();

		char buffer[10];
		int32_t params[2];

		buffer[0] = NET_COMMAND_PACKET;
		buffer[1] = ch;
		switch(ch)
		{
			case 'w':
			case 'W':
				params[0] = STRAIGHT_SPEED;
				params[1] = (ch == 'w')? TIMEOUT_SHORT: TIMEOUT_LONG;
				memcpy(&buffer[2], params, sizeof(params));
				sendData(conn, buffer, sizeof(buffer));
				break;
			case 's':
			case 'S':
				params[0] = STRAIGHT_SPEED;
				params[1] = (ch == 's')? TIMEOUT_SHORT: TIMEOUT_LONG;
				memcpy(&buffer[2], params, sizeof(params));
				sendData(conn, buffer, sizeof(buffer));
				break;
			case 'a':
			case 'A':
				params[0] = TURN_SPEED;
				params[1] = (ch == 'a')? TURN_NUDGE: TURN_FULL;
				memcpy(&buffer[2], params, sizeof(params));
				sendData(conn, buffer, sizeof(buffer));
				break;
			case 'd':
			case 'D':
				params[0] = TURN_SPEED;
				params[1] = (ch == 'd')? TURN_NUDGE: TURN_FULL;
				memcpy(&buffer[2], params, sizeof(params));
				sendData(conn, buffer, sizeof(buffer));
				break;

			case 'c':
			case 'C':
				memcpy(&buffer[2], params, sizeof(params));
				sendData(conn, buffer, sizeof(buffer));
				break;
			case 'f':
			case 'F':
				memcpy(&buffer[2], params, sizeof(params));
				sendData(conn, buffer, sizeof(buffer));
				break;
			case 'q':
			case 'Q':
				quit=1;
				break;
			default:
				printf("BAD COMMAND\n");
		}
	}

	printf("Exiting keyboard thread\n");

	/* TODO: Stop the client loop and call EXIT_THREAD */
	stopClient();
	EXIT_THREAD(conn);
	/* END TODO */

	return NULL;
}

/* TODO: #define filenames for the client private key, certificatea,
   CA filename, etc. that you need to create a client */

#define SERVER_NAME "192.168.137.238"
#define CA_CERT_NAME "signing.pem"
#define PORT_NUM 5001
#define CLIENT_CERT_FNAME "laptop.crt"
#define CLIENT_KEY_FNAME "laptop.key"
#define SERVER_NAME_ON_CERT "weiwei.rocks"

/* END TODO */
void connectToServer(const char *serverName, int portNum)
{
	/* TODO: Create a new client */
	createClient(serverName, portNum, 1, CA_CERT_NAME, SERVER_NAME_ON_CERT, 1, CLIENT_CERT_FNAME, CLIENT_KEY_FNAME, readerThread, writerThread);
	/* END TODO */
}

int main(int ac, char **av)
{
	if(ac != 3)
	{
		fprintf(stderr, "\n\n%s <IP address> <Port Number>\n\n", av[0]);
		exit(-1);
	}

	networkActive = 1;
	connectToServer(av[1], atoi(av[2]));

	/* TODO: Add in while loop to prevent main from exiting while the
	   client loop is running */

	while(client_is_running());

	/* END TODO */
	printf("\nMAIN exiting\n\n");
}
