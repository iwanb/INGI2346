#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "arrays.h"
#include "response.h"

#define MAX_CLIENTS 128

#define NICKNAME_SIZE 16

struct client_set {
	int client_sockets[MAX_CLIENTS];
	char nicknames[MAX_CLIENTS][NICKNAME_SIZE];
	int connected;
} typedef client_set;

client_set *client_init();

int recv_nickname(int socket, char *buffer);

int nickname_at(client_set *clients, char *nickname);

int client_socket_at(client_set *clients, int socket);

int client_nickname_exists(client_set *clients, char *nickname);

int client_connect(client_set *clients, int socket, char *nickname);

void client_quit(client_set *clients, int socket);
