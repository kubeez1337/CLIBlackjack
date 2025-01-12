#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "client.h"
#include "player.h"
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include "gamestate.h"
#include "blackjack.h"

// #include "blackjack.h"

#define MAX_CLIENTS 10

typedef struct {
    int client_socket;
    char name[50];
} Client;


void start_server(const char *server_name, int port);
void handle_client(int client_socket,int server_socket, int shm_id, Player* player, const char* tmp_file, GameState* game_state);
void cleanup_game(GameState *game_state, int shm_id);
void *create_shared_memory(size_t size, key_t key, int *shm_id);

#endif // SERVER_H