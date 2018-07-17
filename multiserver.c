#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h> 
#include <string.h>
#include <unistd.h>
#include <time.h>
#define BUFSIZE 256

//Create a socket
short CreateSocket(void)
{
	short hSocket;

	hSocket = socket(AF_INET, SOCK_STREAM, 0);
	return hSocket;
}

//Receiver thread for server
void* Receiver(void *arg)
{
	int sd =*(int*)arg;
	int numbytes = 1;
	char buffer[BUFSIZE];

	printf("\nConnection established.\n");

	do {
		memset(buffer, 0, BUFSIZE);
		numbytes = recv(sd, buffer, BUFSIZE, 0);
		
		if (numbytes == 0)
		{
			printf("Client disconnected.\n");
			fflush(stdout);
			break;
		}

		printf("Message from client: %s", buffer);

		if(strstr(buffer, "shutdown") != NULL)
		{
			printf("\nServer shutting down...\n");
			system("killall ./server");
			//Terminate the server
		}
		else if(strstr(buffer, "hi") != NULL)
		{
			printf("Hi there!\n\n");
			send(sd, "Hi there!\n", BUFSIZE, 0);
		}	
		else if(strstr(buffer, "bye") != NULL)
		{
			printf("Goodbye!\n\n");
			send(sd, "NULL", BUFSIZE, 0);
		}
		else if(strstr(buffer, "time") != NULL)
		{
			printf("Obtaining local time...\n");
			time_t rawtime = time(NULL);
			struct tm *tm = localtime(&rawtime);
			send(sd, asctime(tm), BUFSIZE,0);
			printf("Local time sent.\n\n");
		}
		else
		{
			while(1)
			{
				printf("Waiting for response...\n");
				fgets(buffer,BUFSIZE,stdin);
				unsigned len = strlen(buffer);
				buffer[len]='\r';
				send(sd,buffer,BUFSIZE,0);
			}
		}
	} while(numbytes > 0);
}

//Sender thread for server
void* Sender(void *arg) 
{
	int sd =*(int*)arg;
	char buffer[BUFSIZE];
	
	while(1)
	{
		memset(buffer,0,BUFSIZE);
		fgets(buffer,BUFSIZE,stdin);
		unsigned len = strlen(buffer);
		buffer[len]='\r';
		send(sd,buffer,BUFSIZE,0);
	}	
}

//Main program
int main()
{
	pthread_t tidReceiver, tidSender;
	
	char server_message[BUFSIZE];
	int server_socket, client_socket, addr_len, PID, pid;
	int iRetval = -1;
	int one = 1;
	int ServerPort = 2222;
	struct sockaddr_in server_address;

	//Create socket
	server_socket = CreateSocket();
	if (server_socket == -1)
	{
		printf("Error creating socket.\n");
		return 1;
	}
	
	// SOL_SOCKET = socket options, one = 1 means enable 
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &one, sizeof(one));

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(ServerPort);
	server_address.sin_addr.s_addr = INADDR_ANY;

	if(bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
	{
		printf("Binding failed.\n");
		return 1;
	}
	
	//Listen for connections, accepts a maximum of 20 simultaneous connections
	listen(server_socket,20);
	printf("Listening for connections...\n");

	while(1)
	{
		addr_len = sizeof(server_address);
		client_socket = accept(server_socket, &server_address, &addr_len);
		
		//Create 2 identical processes
		PID = fork();
		
		//Child process
		if (PID == 0)
		{
			close(server_socket); //Child does not need to listen on server_socket
			memset(server_message, 0, BUFSIZE);
	
			if (pthread_create(&tidReceiver, NULL, &Receiver, &client_socket)) 
			{
				perror("Fail create Receiver thread");
				return 1;
			}
			if (pthread_create(&tidSender, NULL, &Sender, &client_socket)) 
			{
				perror("Fail create Sender thread");
				return 1;
			}
			pthread_join(tidReceiver, NULL);
			pthread_join(tidSender, NULL);

			close(client_socket);
		}
		//Parent process
		else
		{
			close(client_socket); //Parent does not need to listen on client_socket
		}
	}
	return 0;
}
