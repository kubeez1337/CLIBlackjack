#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "player.h"
#include <pthread.h>

typedef struct {
    Player* player;
    Player* dealer;
    // Deck* deck;
    int client_socket;
    int client_count;
    int player_turn;
    int player_bet;
    int game_over;
    pthread_mutex_t mutex;
    pthread_cond_t cond_player, cond_dealer;
    int shm_id;
} GameState;

#endif