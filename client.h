#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "player.h"
#include "gamestate.h"

void start_client(const char *server_name,int port, Player* player);
void *attach_shared_memory(key_t key, size_t size);

#endif 