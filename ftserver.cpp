/***********************************
 * Tom Dale 
 * CS 372
 * Project 2
 * ********************************/

#include <stdio.h>
#include <dirent.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>

#define MAXDATASIZE 12096
#define BUFSIZE 1024


void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int setup(char* portno){
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	int numbytes;
	const char* hostname = "localhost"; 
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	if((rv = getaddrinfo(NULL, portno, &hints, &servinfo)) != 0) {
       		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	        return 1;
	}
	
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
   			 perror("server: socket");
      			 continue;
		}
    		if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
	 		 perror("setsockopt");
 		       	 exit(1);
   		}
   		if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
    			 close(sockfd);
	       		 perror("server: bind");
        		 continue;
  		}
		break;
   	}
	
	if(p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}
 	if(listen(sockfd, 10) == -1){
		perror("listenhasda");
     		exit(1);
   	}	
	
	sin_size = sizeof(their_addr);
   	new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    
	close(sockfd);
	if(new_fd == -1) {
		perror("accept");
    		exit(1);
	}
	
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
//	getnameinfo(get_in_addr((struct sockaddr *p)p->ai_addr), sizeof(sa), hostname, sizeof(hostname), NULL, NULL, 0);
	
	freeaddrinfo(servinfo);
	
	return new_fd; 
}

int datasetup(char* portno, char* hostname) {
	int sockfd;
	char buffer[BUFSIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN]; 
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((rv = getaddrinfo(hostname, portno, &hints, &servinfo)) != 0) {
       		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	        return 1;
	}
	
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
   			perror("data: socket");
      			continue;
		}
    		if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);	
			perror("data: coul not connect");
			continue;
		}
		break;
   	}
	
	if(p == NULL) {
		fprintf(stderr, "data: failed to find address\n");
		exit(1);
	}
 	
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), s, sizeof(s));
	
	freeaddrinfo(servinfo);
	
	return sockfd;
}

void send_directory(int sockfd){
	DIR *pdir;
	struct dirent *pent;

	pdir=opendir(".");

	if (!pdir){
		printf("could not open directory");
		exit(1);
	}	
	errno=0;
	while ((pent=readdir(pdir))){
		send(sockfd, pent->d_name, strlen(pent->d_name), 0);
		send(sockfd, "\n", 1, 0);
	}
	if (errno) {
		printf("read dir failed");
		exit(1); 
	}
	closedir(pdir);
}

void send_file(int sockfd, char* filename){
	char buffer[MAXDATASIZE];
	int numbytes;
	int sentbytes;
	int offset;
	FILE *file = fopen(filename, "rb");
	if (!file) {
		printf("File %s\n not found", filename);
		return;
	} 
	while (!feof(file)){
		numbytes = fread(buffer, 1, sizeof(buffer), file);
		if (numbytes < 1) {
			printf("Can't read from file %s\n", filename);
			fclose(file);
			exit(1);
		}
		offset = 0;
		do {
			sentbytes = send(sockfd, &buffer[offset], numbytes - offset, 0);
			if (sentbytes < 1) {
				printf("Can't write to socket\n");
				fclose(file);
				exit(1);	
			}	

			offset += sentbytes;
		} while (offset < numbytes);
	}

	fclose(file);
	return;	
}

int main(int argc, char *argv[]){
	
	if(argc !=2){
		fprintf(stderr, "Usage: server <port num>\n");
		exit(1);	
	}
	
	printf("Server Running on Port %s\n", argv[1]);
		
	while(1){
		//set up connection
		int mainfd = setup(argv[1]);
	
		//parse message from client
		char buffer[BUFSIZE];
		char* command;
		char* hostname;
		char* dataport;
		char* filename;
		char* temp;
		int numbytes;
	
		memset(buffer, 0, BUFSIZE);
	
		if((numbytes = recv(mainfd, buffer, BUFSIZE - 1, 0)) == -1) {
			perror("recv");
			exit(1);
		}
		buffer[numbytes] = 0;
		
		command = buffer;
		temp = strchr(buffer, ' ');
		*temp = 0;
	
		hostname = temp+1;
		temp = strchr(hostname, ' ');
		*temp = 0;
	
		dataport = temp+1;
		temp = strchr(dataport, ' ');
		*temp = 0;
	
		filename = temp+1;
		temp = strchr(filename, ' ');
	
		close(mainfd);
	
		//connect to client
	
		int datafd = datasetup(dataport, hostname);
		
		if(strcmp(command, "-l") == 0) {
			printf("Directory List Requested on Port %s\n", dataport); 
			send_directory(datafd);
		}
		else if(strcmp(command, "-g") == 0) {
			printf("File '%s' Requested on Port %s\n", filename, dataport);  
			send_file(datafd, filename);
		}	
		else {
			char* m = "Invalid Command"; 
			send(datafd, m, strlen(m), 0);  
		}
		close(datafd);
	}
	return 0;
}	
