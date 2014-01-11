#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <limits.h>
#include <netdb.h>
#include "arrays.h"
#include "request.h"
#include "response.h"
#include "client.h"

#define BUFFER_SIZE 1024

int print_socket_string(int sockfd) {
	char byte;
	do {
		if (recv(sockfd, &byte, 1, MSG_WAITALL) < 0) {
			perror("Error receiving");
			return -1;
		}
		putchar(byte);
	} while (byte != '\0');
	return 0;
}

int main(int argc, char *argv[]) {
	int sockfd;
    unsigned int portno = 6666;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int active = 1;
    
    int i;
    
    int connected;
    
    int selected;
    int received;
    
    // Set of file descriptors we want to read from: server or stdin
	fd_set waiting_set;
	// Set used when calling pselect
	fd_set selected_set;
	
	FD_ZERO(&waiting_set);
    
    char request, response;
    
    char *nickname;
    
    char other_nickname[NICKNAME_SIZE];
    char buffer[BUFFER_SIZE];
    
    if (argc < 3) {
      printf("Not enough arguments, please provide hostname and nickname\n");
      return EXIT_FAILURE;
    }
    
    nickname = argv[2];
    
    if (argc > 3) {
        /*parse arg : port nb*/
        portno = strtol(argv[3], NULL, 10);
    } else {
        portno = 6666;
    }
    
    /*resolution hostname*/ 
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        return EXIT_FAILURE;
    }

    /*creation socket client*/
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("can't create socket");
        return EXIT_FAILURE;
	}
    /*creation socket info server*/ 
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    /*make connection between client and server*/
    if (connect(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        perror("ERROR connecting to server");
        active = 0;
    }
    
    FD_SET(0, &waiting_set);
    FD_SET(sockfd, &waiting_set);
    
    request = REQUEST_CONNECT;
    // Connect to IRC server
    send(sockfd, &request, 1, 0);
    send(sockfd, nickname, strlen(nickname)+1, 0);
    
    received = recv(sockfd, &response, 1, MSG_WAITALL);
    if (received < 0) {
		perror("Error recv");
		return EXIT_FAILURE;
	}
    
    if (response < 0) {
		print_response_error(response, "Error on connect");
		close(sockfd);
		return EXIT_FAILURE;
	}
	
	printf("Connected to server as '%s'\n", nickname);
	
	while (active) {
		selected_set = waiting_set;
		selected = pselect(INT_MAX, &selected_set, NULL, NULL, NULL, NULL);
		if (selected < 0) {
			// pselect returned an error
			if (errno == EINTR) {
				// pselect was interrupted, try again
				continue;
			}
			perror("Error on select");
			active = 0;
			break;
		}
		if (selected == 0) {
			continue;
		}
		
		if (FD_ISSET(0, &selected_set)) {
			// Process command
			bzero(buffer, BUFFER_SIZE);
			fgets(buffer, BUFFER_SIZE-1, stdin);
			if (strcmp(buffer, "quit\n") == 0) {
				break;
			}
			
			if (strcmp(buffer, "who\n") == 0) {
				request = REQUEST_WHO;
				send(sockfd, &request, 1, 0);
				received = recv(sockfd, &response, 1, MSG_WAITALL);
				
				if (response == 0) {
					received = recv(sockfd, &connected, sizeof(connected), MSG_WAITALL);
					connected = ntohl(connected);
					printf("%d persons are connected:\n", connected);
					i = 0;
					while (i < connected) {
						if (print_socket_string(sockfd) < 0) return EXIT_FAILURE;
						putchar('\n');
						i++;
					}
				}
			}
			
			if (strcmp(buffer, "msg\n") == 0) {
				printf("To who ?\n");
				bzero(other_nickname, NICKNAME_SIZE);
				fgets(other_nickname, NICKNAME_SIZE-1, stdin);
				if (other_nickname[strlen(other_nickname)-1] == '\n') {
					other_nickname[strlen(other_nickname)-1] = '\0';
				}
				printf("Message ?\n");
				// Reuse buffer
				bzero(buffer, BUFFER_SIZE);
				fgets(buffer, BUFFER_SIZE-1, stdin);
				if (buffer[strlen(buffer)-1] == '\n') {
					buffer[strlen(buffer)-1] = '\0';
				}
				
				// Send everything:
				request = REQUEST_MSG;
				send(sockfd, &request, 1, 0);
				send(sockfd, other_nickname, strlen(other_nickname)+1, 0);
				send(sockfd, buffer, strlen(buffer)+1, 0);
				
				received = recv(sockfd, &response, 1, MSG_WAITALL);
				if (response < 0) {
					print_response_error(response, "Error on msg");
				}
			}
		}
		
		if (FD_ISSET(sockfd, &selected_set)) {
			// Process async msg
			received = recv(sockfd, &response, 1, MSG_WAITALL);
			if (response == ASYNC_MSG) {
				recv_nickname(sockfd, other_nickname);
				printf("Message received from '%s':\n", other_nickname);
				
				if (print_socket_string(sockfd) < 0) return EXIT_FAILURE;
				
				putchar('\n');
			} else {
				printf("Unexpected async response from server\n");
			}
		}
	}
	
	printf("Disconnecting ...\n");
	request = REQUEST_QUIT;
	send(sockfd, &request, 1, 0);
	received = recv(sockfd, &response, 1, MSG_WAITALL);
	
	close(sockfd);
    
    return EXIT_SUCCESS;
}
