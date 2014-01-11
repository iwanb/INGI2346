#include "client.h"

client_set *client_init() {
	client_set *clients = calloc(1, sizeof(client_set));
	return clients;
}

int recv_nickname(int socket, char *buffer) {
	char byte;
	int i = 0;
	do {
		if (recv(socket, &byte, 1, MSG_WAITALL) < 0) {
			return -1;
		}
		buffer[i] = byte;
		i++;
	} while (i < NICKNAME_SIZE && byte != '\0');
	
	if (i == NICKNAME_SIZE && byte != '\0') {
		buffer[NICKNAME_SIZE-1] = '\0';
		return 0;
	}
	
	return i;
}

int nickname_at(client_set *clients, char *nickname) {
	int i = 0;
	while (i < clients->connected) {
		if (strcmp(nickname, clients->nicknames[i]) == 0) {
			return i;
		}
		i++;
	}
	
	return i;
}

int client_socket_at(client_set *clients, int socket) {
	return is_at(socket, clients->client_sockets, clients->connected);
}

int client_nickname_exists(client_set *clients, char *nickname) {
	return (nickname_at(clients, nickname) < clients->connected);
}


int client_connect(client_set *clients, int socket, char *nickname) {
	if (clients->connected == MAX_CLIENTS - 1) {
		return TOO_MANY_CONNECTIONS;
	}
	
	clients->client_sockets[clients->connected] = socket;
	
	strncpy(clients->nicknames[clients->connected], nickname, NICKNAME_SIZE-1);
	
	if (client_nickname_exists(clients, clients->nicknames[clients->connected])) {
		return NICKNAME_ALREADY_IN_USE;
	}
	
	clients->connected++;
	
	return 0;
}

void client_quit(client_set *clients, int socket) {
	int position = is_at(socket, clients->client_sockets, clients->connected);
	
	remove_from(position, clients->client_sockets, clients->connected);
	
	while (position < clients->connected - 1) {
		strcpy(clients->nicknames[position], clients->nicknames[position+1]);
		position++;
	}
	
	clients->connected--;
}
