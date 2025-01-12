#ifndef PLAYER_H
#define PLAYER_H
#include "card.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int client_socket;
    char name[50];
    int chips;
    Card* cards[11];
    int card_count;
    int card_sum;
} Player;

#endif 