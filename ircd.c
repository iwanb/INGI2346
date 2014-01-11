#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <limits.h>
#include <signal.h>
#include "arrays.h"
#include "request.h"
#include "response.h"
#include "client.h"

#define BUFFER_SIZE 128

int alive = 1;

client_set * clients;

int ignore_string_argument(int socket) {
	char byte;
	do {
		if (recv(socket, &byte, 1, MSG_WAITALL) < 0) {
			return -1;
		}
	} while (byte != '\0');
	return 0;
}

// Return 0 if socket should be closed
// -1 for error
int process_request(char request, int socket) {
	char byte;
	int i;
	int from_client_id;
	int client_id;
	int to_socket;
	
	int connected;
	
	char response = 0;
	
	char buffer[BUFFER_SIZE];
	char nickname[NICKNAME_SIZE];
	bzero((void *) buffer, BUFFER_SIZE);
	bzero((void *) nickname, NICKNAME_SIZE);
	
	if (request == REQUEST_CONNECT) {
		i = recv_nickname(socket, nickname);
		
		if (i < 0) {
			return -1;
		}
		
		if (i == 0) {
			response = NICKNAME_TOO_LONG;
			send(socket, &response, 1, 0);
			// Close socket
			return 0;
		}
		response = client_connect(clients, socket, nickname);
		
		send(socket, &response, 1, 0);
		
		return 1;
	}
	
	if (request == REQUEST_QUIT) {
		client_quit(clients, socket);
		
		send(socket, &response, 1, 0);
		
		return 0;
	}
	
	if (request == REQUEST_MSG) {
		i = recv_nickname(socket, nickname);
		
		if (i < 0) {
			return -1;
		}
		
		if (i == 0) {
			response = NICKNAME_TOO_LONG;
			send(socket, &response, 1, 0);
			// Close socket
			return 0;
		}
		
		if ((client_id = nickname_at(clients, nickname)) >= clients->connected) {
			response = UNKNOWN_NICKNAME;
			
			if (ignore_string_argument(socket) < 0) {
				return -1;
			}
			
			send(socket, &response, 1, 0);
			return 1;
		}
		
		// Forward message to recipient
		to_socket = clients->client_sockets[client_id];
		
		from_client_id = client_socket_at(clients, socket);
		
		byte = ASYNC_MSG;
		send(to_socket, &byte, 1, 0);
		send(to_socket, clients->nicknames[from_client_id], strlen(nickname)+1, 0);
		do {
			if (recv(socket, &byte, 1, MSG_WAITALL) < 0) {
				return -1;
			}
			send(to_socket, &byte, 1, 0);
		} while (byte != '\0');
		
		send(socket, &response, 1, 0);
		return 1;
	}
	
	if (request == REQUEST_WHO) {
		send(socket, &response, 1, 0);
		
		connected = clients->connected;
		connected = htonl(connected);
		
		send(socket, &connected, sizeof(connected), 0);
		
		for (i = 0; i < clients->connected; i++) {
			send(socket, clients->nicknames[i], strlen(clients->nicknames[i])+1, 0);
		}
		
		return 1;
	}
	
	// Unknown request, closing socket
	return 0;
}

/* ctrl+c handler */
void
termination_handler (int signum)
{
    // Terminate
    alive = 0;
}

int main(int argc, char *argv[]) {
	int port_no = 6666;
	// Server listening socket
	int sockd_wait;
	// Client sockets
	int client_sockets[MAX_CLIENTS];
	// Number of connected clients
	int connected = 0;
	
	int i;
	
	char request;
	ssize_t actually_read;
	
	// Set of file descriptors (sockets) we want to read from
	fd_set waiting_set;
	// Set used when calling pselect
	fd_set selected_set;
	
	int selected;
	
	struct sockaddr_in server_address;
	
	clients = client_init();
	
	FD_ZERO(&waiting_set);
	
	// Create listening socket
	sockd_wait = socket(AF_INET, SOCK_STREAM, 0);
    if (sockd_wait < 0) {
        perror("ERROR opening socket");
        return EXIT_FAILURE;
    }
    
    // Receive from anyone
    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port_no);
    
    // Bind the socket
    if (bind(sockd_wait, (struct sockaddr *) &server_address,
                sizeof(server_address)) < 0) {
        perror("ERROR on binding");
        close(sockd_wait);
        return EXIT_FAILURE;
    }
    
    if (listen(sockd_wait, 5) < 0) {
		perror("ERROR on listen");
        close(sockd_wait);
        return EXIT_FAILURE;
	}
	
	// Wait for new clients (accept)
	FD_SET(sockd_wait, &waiting_set);
	
	if (signal (SIGINT, termination_handler) == SIG_IGN) {
        // Ctrl+c handler to terminate early
        signal (SIGINT, SIG_IGN);
    }
    
    while (alive) {
		selected_set = waiting_set;
		selected = pselect(INT_MAX, &selected_set, NULL, NULL, NULL, NULL);
		if (selected < 0) {
			// pselect returned an error
			if (errno == EINTR) {
				// pselect was interrupted, try again
				continue;
			}
			perror("Error when waiting to receive from sockets");
			alive = 0;
			break;
		}
		if (selected == 0) {
			continue;
		}
		
		if (FD_ISSET(sockd_wait, &selected_set)) {
			client_sockets[connected] = accept(sockd_wait, NULL, NULL);
			if (client_sockets[connected] < 0) {
				// Client could not connect
				perror("ERROR on accept");
				continue;
			}
			
			// We have a new client
			FD_SET(client_sockets[connected], &waiting_set);
			connected++;
		}
		
		for (i = 0; i < connected; i++) {
			if (FD_ISSET(client_sockets[i], &selected_set)) {
				// Got a message from a client, process it
				actually_read = recv(client_sockets[i], &request, 1, MSG_WAITALL);
				if (actually_read < 0) {
					perror("Error reading from client socket, closing socket");
					close(client_sockets[i]);
					remove_from(i, client_sockets, connected);
					connected--;
					break;
				}
				
				if (process_request(request, client_sockets[i]) < 1) {
					close(client_sockets[i]);
					FD_CLR(client_sockets[i], &waiting_set);
				}
			}
		}
	}
	
	for (i = 0; i < connected; i++) {
		close(client_sockets[i]);
	}
	
	close(sockd_wait);
    
    return EXIT_SUCCESS;
}
