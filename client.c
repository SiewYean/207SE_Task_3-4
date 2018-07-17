#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define BUFSIZE 256
 
 
//Create a Socket for server communication
short CreateSocket(void)
{
        short hSocket;

        hSocket = socket(AF_INET, SOCK_STREAM, 0);
        return hSocket;
}
 
//try to connect with server
int ConnectSocket(int hSocket)
{
        int iRetval=-1;
        int ServerPort = 2222;
        struct sockaddr_in remote={0};
 
        remote.sin_addr.s_addr = inet_addr("127.0.0.1"); //Local Host
        remote.sin_family = AF_INET;
        remote.sin_port = htons(ServerPort);
 
        iRetval = connect(hSocket , (struct sockaddr *)&remote , sizeof(struct sockaddr_in));
 
        return iRetval;
}
 
 
// Send the data to the server and set the timeout of 20 seconds
int SendData(int hSocket,char* Rqst,short lenRqst)
{
	int shortRetval;
        shortRetval = send(hSocket , Rqst , lenRqst , 0);
 
        return shortRetval;
}
 
 
//receive the data from the server 
int ReceiveData(int hSocket,char* Rsp,short RvcSize)
{
	int shortRetval;
	shortRetval = recv(hSocket, Rsp, RvcSize, 0);
 
	return shortRetval;
}
 
 
//main driver program
int main(int argc , char *argv[])
{
	int client_socket, read_size;
	struct sockaddr_in server;
	char SendToServer[BUFSIZE];
	char server_reply[BUFSIZE];
 
	//Create socket
	client_socket = CreateSocket();
	
	if(client_socket == -1)
	{
		printf("Error creating socket.\n");
		return 1;
	}
 
	//Connect to remote server
	if (ConnectSocket(client_socket) < 0)
	{
		perror("Failed to establish connection.\n");
		return 1;
	}

	printf("Sucessfully conected with server\n");

	while(1)
	{
		printf("\nMessage to server: ");

		fgets(SendToServer, sizeof(SendToServer), stdin);
 
		//Send data to the server
		SendData(client_socket, SendToServer, strlen(SendToServer));
 
    		//Received the data from the server
		read_size = ReceiveData(client_socket, server_reply, BUFSIZE);
		
		if (strstr(server_reply, "NULL") != NULL)
		{
			read_size = 0;
		}

		if (read_size > 0)
		{
			printf("Server response: %s", server_reply);
		}
		else
		{
			if (strstr(server_reply, "NULL") == NULL)
			{
				printf("\nServer is offline or not responding.\n");
			}
			printf("Breaking off connection...\n");
			close(client_socket);
			exit(0);
		}
		memset(SendToServer, 0, BUFSIZE);
		memset(server_reply, 0, BUFSIZE);
	}
	return 0;
}
