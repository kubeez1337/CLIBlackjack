#ifndef BLACKJACK_H
#define BLACKJACK_H

#include "deck.h"
#include "player.h"
#include "server.h"
#include "gamestate.h"
void print_out_dealer_first(Player *player, int client_socket);
void print_out_player_cards(Player *player, int client_socket);
void *dealer_turn(void *arg);
void *player_turn(void *arg);
void start_blackjack_game(int client_socket, GameState *game_state, Player *player);
int calculate_hand_value(Player *player);
Card* drawCardGame(GameState *game_state);
#endif // BLACKJACK_H