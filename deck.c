#include "deck.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>

static Deck deck;

void initializeDeck() {
    const char *values[] = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"};
    const char *suits[] = {"H", "D", "C", "S"};
    int index = 0;

    for (int i = 0; i < 13; i++) {
        for (int j = 0; j < 4; j++) {
            deck.deck[index] = (Card *)malloc(sizeof(Card));
            strcpy(deck.deck[index]->value, values[i]);
            strcpy(deck.deck[index]->suit, suits[j]);
            index++;
        }
    }
    deck.card_index = 0;
}

void shuffleDeck() {
    srand(time(NULL));
    for (int i = 0; i < 52; i++) {
        int j = rand() % 52;
        Card *temp = deck.deck[i];
        deck.deck[i] = deck.deck[j];
        deck.deck[j] = temp;
    }
}

Card* drawCard() {
    if (deck.card_index < 52) {
        return deck.deck[deck.card_index++];
    } else {
        //cleanCards();
        return NULL;
    }
}

void cleanCards(){
    for (int i = 0; i < 52; i++) {
        if (deck.deck[i] != NULL) {
            free(deck.deck[i]);
            deck.deck[i] = NULL;
        }
    }
    //free(*deck.deck);
}