#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>

#define DEFAULT_PORT 3380
#define MAXMESSAGESIZE 200

int sockfd;
pthread_t read_tid;
pthread_t write_tid;
char Message[MAXMESSAGESIZE];

void* ReadThread(void* arg){
	int recv_retval = recv(sockfd,Message,sizeof(Message),0);
	if(recv_retval == 0 || recv_retval == -1){
		perror("Connection lost or Read failed:");
		pthread_exit(NULL);
	}
	printf("The result is:\n %s\n",Message);
return NULL;
}

void* WriteThread(void* arg){
	printf("Please enter the expression:\n");
	fgets(Message,sizeof(Message),stdin);
	int send_retval = send(sockfd,Message,strlen(Message),0);
	if(send_retval == 0 || send_retval == -1){
		perror("Connection lost or Write failed:\n");
		pthread_exit(NULL);
	}
	printf("Please enter the values:\n");
	memset(Message,0,sizeof(Message));
	fgets(Message,sizeof(Message),stdin);
	send_retval = send(sockfd,Message,strlen(Message),0);
	if(send_retval == 0 || send_retval == -1){
		perror("Connection lost or Write failed:\n");
		pthread_exit(NULL);
	}
return NULL;
}

int main(){
	printf("Client start working:\n");
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    int optval = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0){
		perror("Seting socket option REUSEADDR failed:\n");
		exit(EXIT_FAILURE);
	}
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) < 0){
		perror("Setting socket option REUSEPORT failed:\n");
	}
	
	struct sockaddr_in client;
	client.sin_family = AF_INET;
    client.sin_port = htons(DEFAULT_PORT);
    client.sin_addr.s_addr = inet_addr("127.0.0.1");
	printf("Waiting connection...\n");
    if(connect(sockfd, (struct sockaddr*)&client, sizeof(client)) < 0){
		perror("Connection failed:\n");
		close(sockfd);
		exit(EXIT_FAILURE);
    }
	printf("Connected to server:\n");
	if(pthread_create(&write_tid,NULL,WriteThread,NULL)){
		perror("Write thread creation failed:\n");
		exit(EXIT_FAILURE);
	}
	if(pthread_create(&read_tid,NULL,ReadThread,NULL)){
		perror("Read thread creation failed:\n");
		exit(EXIT_FAILURE);
	}
	pthread_join(read_tid,NULL);
	pthread_join(write_tid,NULL);
	printf("Client finish:\n");
return 0;
}
