#ifndef DECK_H
#define DECK_H

#include "card.h"
#include <time.h>

typedef struct {
    Card* deck[52];
    int card_index;
} Deck;

void initializeDeck();
void shuffleDeck();
Card* drawCard();
void cleanCards();
#endif 