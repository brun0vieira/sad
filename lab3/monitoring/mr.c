#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib") // Winsock Lib

int main()
{
    // Declare variables and structures
    HANDLE hSerial;
    DCB dcbSerialParams = {0};
    COMMTIMEOUTS timeouts = {0};

    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server;
    char msg[300];

    printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		printf("Failed. Error Code : %d",WSAGetLastError());
		return 1;
	}
	
	printf("Initialised.\n");
         
    // Open the highest available serial port number
    fprintf(stderr, "Opening serial port...");
    hSerial = CreateFile(
                "\\\\.\\COM2", GENERIC_READ|GENERIC_WRITE, 0, NULL,
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

    if (hSerial == INVALID_HANDLE_VALUE)
    {
            fprintf(stderr, "Error\n");
            return 1;
    }
    else fprintf(stderr, "OK\n");
     
    // Set device parameters (9600 baud, 1 start bit, 1 stop bit, no parity)
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (GetCommState(hSerial, &dcbSerialParams) == 0)
    {
        fprintf(stderr, "Error getting device state\n");
        CloseHandle(hSerial);
        return 1;
    }
     
    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    if(SetCommState(hSerial, &dcbSerialParams) == 0)
    {
        fprintf(stderr, "Error setting device parameters\n");
        CloseHandle(hSerial);
        return 1;
    }
 
    // Set COM port timeout settings
    // Reference: https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddser/ns-ntddser-_serial_timeouts
    timeouts.ReadIntervalTimeout = 500;
    timeouts.ReadTotalTimeoutConstant = 500;
    timeouts.ReadTotalTimeoutMultiplier = 100;
    timeouts.WriteTotalTimeoutConstant = 500;
    timeouts.WriteTotalTimeoutMultiplier = 50;
    if(SetCommTimeouts(hSerial, &timeouts) == 0)
    {
        fprintf(stderr, "Error setting timeouts\n");
        CloseHandle(hSerial);
        return 1;
    }
 
    // while...

    // connect to remote server...

    // receive data from com

    // send data to server

    // close connection (socket)

    // Close serial port

    // Test

    while(1)
    {
        // Reference: https://www.binarytides.com/winsock-socket-programming-tutorial/
        if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
            printf("Could not create socket : %d" , WSAGetLastError());

	    printf("Socket created.\n");

        server.sin_addr.s_addr = inet_addr("193.136.120.133");
        server.sin_family = AF_INET;
        server.sin_port = htons(80); 

        //Connect to remote server
		if (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0)
		{
			puts("connect error\n");
			return 1;
		}
		puts("Connected\n");

        // Send specified text (remaining command line arguments)
		DWORD bytes_ridden = 0;
		char message[250] = {0};
		fprintf(stderr, "Waiting to receive bytes...\n");
		while(bytes_ridden==0){
			if(!ReadFile(hSerial, message, 250, &bytes_ridden, NULL))
			{
				fprintf(stderr, "Error\n");
				CloseHandle(hSerial);
				return 1;
			}
		}
		fprintf(stderr, "%d bytes ridden\n", bytes_ridden);

        fprintf(stderr, "%s\n\n", message);

    }

    fprintf(stderr, "Closing serial port...");
    if (CloseHandle(hSerial) == 0)
    {
        fprintf(stderr, "Error\n");
        return 1;
    }
    fprintf(stderr, "OK\n");
    
    // exit normally
    return 0;
}