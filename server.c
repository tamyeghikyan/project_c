#include <stdio.h>		//server
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define DEFAULT_PORT 3380
#define MAXCOMMANDSIZE 200
#define BUFSIZE 50
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t tid;
int accept_fd;
int scope_exist=0;
char buffer[MAXCOMMANDSIZE];
bool check_scopes();
char scopebuf[BUFSIZE];
int scope_execute;
bool flag = true;
char* error_expression = "Invalid arguments:";
char values[BUFSIZE];

void add_values(){
	char* tmp = values;
	while(*tmp != '\n' && *tmp != '\0'){
	char value1 = *tmp;
	char* tmp1;
	
		while((tmp1 = strchr(buffer,value1)) != NULL){
			*tmp1 = *(tmp+1);
			
		}
		tmp += 2;
	}
}
void remove_spaces(int val){
	char* tmp;
	if(val == 0){
		tmp = buffer;
		
	}
	else{
		
		tmp = scopebuf;
	}
    printf("brfore remove %s\n",buffer);
	char buf1[BUFSIZE];
	memset(buf1,0,sizeof(buf1));
	int j = 0;
	while(*tmp != '\n' && *tmp != '\0'){
	if(*tmp != ' '){
		buf1[j] = *tmp;
		j++;
	}
	++tmp;
	}
	printf("removed spaces %s\n",buf1);
	if(val == 0){
		memset(buffer,0,sizeof(buffer));
		strcpy(buffer, buf1);
		
	}
	else{
		memset(scopebuf,0,sizeof(scopebuf));
		strcpy(scopebuf, buf1);
		
	}
}
bool execute_expression(int val){
	
    char* tmp;
    if(val == 0){
        tmp = buffer;
    }
    else{
        tmp = scopebuf;
    }
	
    while(strlen(tmp) > 1){
	
	char *ch;
        if(ch = strchr(tmp, '!')){
            *ch = (*(ch + 1) == 48) ? '1' : '0' ;
            *(ch+1) = ' ';	
            remove_spaces(0);
        }else if(ch = strchr(tmp, '&')){
            *(ch-1) = ((*(ch-1)-'0') && ((*(ch+2)) - '0')) + '0';
            *ch = ' ';
            *(ch+1) = ' ';
            *(ch+2) = ' ';
            remove_spaces(0);
        }
        else if(ch = strchr(tmp, '^')){
            *(ch-1) = ((*(ch-1)) -'0' ^ ((*(ch+1))) - '0') + '0';
            printf("%s\n", ch);
            *ch = ' ';
            *(ch+1) = ' ';
            remove_spaces(0);
        }
        else if(ch = strchr(tmp, '|')){
            *(ch-1) = ((*(ch-1)-'0') || ((*(ch+2)) - '0')) + '0';
            *ch = ' ';
            *(ch+1) = ' ';
            *(ch+2) = ' ';
            remove_spaces(0);
        }
        else {
			printf("Execute\n");
            break;
			}
		
    }
	
	printf("The result is %s\n", tmp);
	return *tmp;
}


bool scope_find(){
        char* open = strchr(buffer, '(');
        char* buf = strchr(buffer, '(');
        char* tmp = strchr(buffer, '(');
        memset(scopebuf, 0, sizeof(scopebuf));
       	int index = 0;
        ++open;
        while(*open != ')'){
            scopebuf[index] = *open;
            ++open;
            ++index;
        }
       	bool value = execute_expression(0);////AYAYAY
		if(value == true){
			*buf = '1';
		}
		else{
			*buf = '0';
		}
		++buf;
		while(*buf != ')'){
			*buf = ' ';
            ++buf;
		}  
        *buf = ' ';
        printf("after execution %s\n",buffer);
		remove_spaces(0); 
        printf(" scope find is   %s\n",buffer);
return true;
}



bool check_scopes() {
    scope_execute = 0;
    buffer[strlen(buffer)] = '\n';
    int openScope = 0;
    char* tmp = buffer;
   while(*tmp != '\n'){
        if(*tmp == '('){
            if(*(tmp+1) == ')'){
                return false;
            }
            scope_execute+=1;
            ++openScope;
        }
        if(openScope != 0 && openScope != 1){
            return false;
    }
    if(*tmp == ')'){
        --openScope;
    }
    if(openScope != 0 && openScope != 1){
        return false;
    }
    ++tmp;
   }
   if(openScope == 0){
        return true;
    }else
    return false;
}


bool check_tokens(){
    buffer[strlen(buffer)] = '\n';
    char *tmp = buffer;
    while(*tmp != '\n'){
        if(*tmp == '('){
            ++tmp;
            continue;
        }else if(*tmp == ')'){
            ++tmp;
            continue;
        }else if(*tmp >= 'A' && *tmp <= 'Z'){
            ++tmp;
            continue;
        }else if(*tmp == '&' && *(tmp + 1) == '&'){
            tmp += 2;
            continue;
        }else if(*tmp == '|' && *(tmp +1) == '|'){
            tmp += 2;
            continue;
        }else if(*tmp == '^'){
            ++tmp;
            continue;
        }else if(*tmp == '!'){
            ++tmp;
            continue;
        }else{
        return false;
        }
    }
	
    return true;
}

void* ThreadFunc(void* arg){
	pthread_mutex_lock(&mutex);
	memset(buffer,0,sizeof(buffer));
	int recv_retval = recv(accept_fd,buffer,sizeof(buffer),0);
	if(recv_retval == 0 || recv_retval == -1){
		perror("Receave failed or connection lost:\n");
		exit(EXIT_FAILURE);
	}
	
	memset(values,0,sizeof(values));
	recv_retval = recv(accept_fd,values,sizeof(values),0);
	if(recv_retval == 0 || recv_retval == -1){
		perror("Receave failed or connection lost:\n");
		exit(EXIT_FAILURE);
	}
	
	remove_spaces(0);
	
	if(check_tokens() && check_scopes()){
		
		add_values();
		
		while(scope_execute){
			scope_find();
			printf("The scope executed:\n");
			--scope_execute;
		}
		printf("Final execution start:\n");
		execute_expression(0);
		printf("Execution ends:\n");
	}
	else{
		memset(buffer,0,sizeof(buffer));
		strcat(buffer,error_expression);	
	}
	buffer[1] = 0;
	int send_retval = send(accept_fd,buffer,2,0);
	if(send_retval == 0 || send_retval == -1){
		perror("Send failed or connection lost");
		pthread_exit(NULL);
	}
printf("Messege sent successfully:\n");		
	pthread_mutex_unlock(&mutex);
return NULL;
}


int main(){
	printf("Server start working:\n");
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("Socket creation failed:");
        exit(EXIT_FAILURE);
    }
    int optval = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(optval)) < 0){
		perror("Setting socket option REUSEADDR failed:\n");
		exit(EXIT_FAILURE);
	}
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const void*)&optval, sizeof(optval)) < 0){
		perror("Setting socket option REUSEPORT failed:");
		exit(EXIT_FAILURE);
	}
    struct sockaddr_in server;
    memset((void*)&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(DEFAULT_PORT);
    server.sin_addr.s_addr = INADDR_ANY;
    if(bind(sockfd, (const struct sockaddr*)&server, sizeof(server)) < 0){
        perror("Socket binding failed:\n");
        exit(EXIT_FAILURE);
    }
    if(listen(sockfd,1) == -1){
        perror("Socket listening failed:\n");
        exit(EXIT_FAILURE);
    }
	accept_fd = accept(sockfd, NULL, NULL);
	if(accept_fd == -1){
		perror("Connection failed:\n");
		exit(EXIT_FAILURE);
	}
	printf("Client connected to server:\n");
	if(pthread_create(&tid,NULL,ThreadFunc,NULL)){
		perror("Thread creation failed:\n");
		exit(EXIT_FAILURE);	
	}
	pthread_join(tid,NULL);
	printf("Server ends:\n");


}
