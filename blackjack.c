#include "blackjack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int calculate_first_dealer_value(Player *player){
    if (player->card_count == 2){
        if (strcmp(player->cards[0]->value, "A") == 0){
            return 11;
        } else if (strcmp(player->cards[0]->value, "K") == 0 || 
                   strcmp(player->cards[0]->value, "Q") == 0 || 
                   strcmp(player->cards[0]->value, "J") == 0) {
            return 10;
        } else {
            return atoi(player->cards[0]->value);
        }
    }

}

int calculate_hand_value(Player *player) {
    int value = 0;
    int aces = 0;

    for (int i = 0; i < player->card_count; i++) {
        if (strcmp(player->cards[i]->value, "A") == 0) {
            aces++;
            value += 11;
        } else if (strcmp(player->cards[i]->value, "K") == 0 || 
                   strcmp(player->cards[i]->value, "Q") == 0 || 
                   strcmp(player->cards[i]->value, "J") == 0) {
            value += 10;
        } else {
            value += atoi(player->cards[i]->value);
        }
    }

    while (value > 21 && aces > 0) {
        value -= 10;
        aces--;
    }
    player->card_sum = value;
    return value;
}
void print_out_dealer_first(Player *player, int client_socket) {
    char buffer_out[1024];
    int res = calculate_first_dealer_value(player);
    snprintf(buffer_out, sizeof(buffer_out), "Dealer: %s|%s, ?\nSum: %d\n", player->cards[0]->value, player->cards[0]->suit, res);
    write(client_socket, buffer_out, strlen(buffer_out));
}

void print_out_separators(int client_socket) {
    char buffer_out[1024];
    snprintf(buffer_out, sizeof(buffer_out), "--------------------------------\n");
    write(client_socket, buffer_out, strlen(buffer_out));
}

void print_out_player_cards(Player *player, int client_socket) {
    char buffer_out[1024];
    snprintf(buffer_out, sizeof(buffer_out), "%s: ",player->name);
    write(client_socket, buffer_out, strlen(buffer_out));
    for (int i = 0; i < player->card_count; i++) {
        snprintf(buffer_out, sizeof(buffer_out), "%s|%s, ", player->cards[i]->value, player->cards[i]->suit);
        write(client_socket, buffer_out, strlen(buffer_out));
    }
    int res = calculate_hand_value(player);
    snprintf(buffer_out, sizeof(buffer_out), "\n%s Sum: %d\n", player->name, res);
    write(client_socket, buffer_out, strlen(buffer_out));
    print_out_separators(client_socket);
}

Card* drawCardGame(GameState *game_state) {
    char buffer_out[1024];
    Card* card = drawCard();
    if (card == NULL){
        snprintf(buffer_out, sizeof(buffer_out), "No more cards in the deck.\n");
        write(game_state->client_socket, buffer_out, strlen(buffer_out));
        close(game_state->client_socket);
        game_state->game_over = 1;
        
        pthread_exit(NULL);
    }
    return card;
}

void *dealer_turn(void *arg) {
    GameState *game_state = (GameState *)arg;
    char buffer_out[1024];

    pthread_mutex_lock(&game_state->mutex);
    while (game_state->player_turn != 0 && game_state->player->card_sum <= 21) {
        pthread_cond_wait(&game_state->cond_dealer, &game_state->mutex);
    }
    if (calculate_hand_value(game_state->dealer) >= 17){
        snprintf(buffer_out, sizeof(buffer_out), "Dealer stands and reveals second card\n");
        write(game_state->client_socket, buffer_out, strlen(buffer_out));
        print_out_player_cards(game_state->dealer, game_state->client_socket);
        snprintf(buffer_out,sizeof(buffer_out),"\n");
        write(game_state->client_socket, buffer_out, strlen(buffer_out));
        print_out_player_cards(game_state->player, game_state->client_socket);
    }
    while (calculate_hand_value(game_state->dealer) < 17) {
        sleep(2);
        snprintf(buffer_out, sizeof(buffer_out), "Dealer hits\n");
        write(game_state->client_socket, buffer_out, strlen(buffer_out));
        
        game_state->dealer->cards[game_state->dealer->card_count++] = drawCardGame(game_state);
        print_out_separators(game_state->client_socket);
        print_out_player_cards(game_state->dealer, game_state->client_socket);

        print_out_player_cards(game_state->player, game_state->client_socket);
        print_out_separators(game_state->client_socket);
        
    }

    int player_value = calculate_hand_value(game_state->player);
    int dealer_value = calculate_hand_value(game_state->dealer);

    if (player_value > 21) {
        snprintf(buffer_out, sizeof(buffer_out), "You bust! Dealer wins.\n");
    } else if (dealer_value > 21 || player_value > dealer_value) {
        snprintf(buffer_out, sizeof(buffer_out), "You win!\n");
        game_state->player->chips += game_state->player_bet * 2;
    } else if (player_value < dealer_value) {
        snprintf(buffer_out, sizeof(buffer_out), "Dealer wins.\n");
    } else {
        snprintf(buffer_out, sizeof(buffer_out), "It's a tie!\n");
        game_state->player->chips += game_state->player_bet;
    }
    write(game_state->client_socket, buffer_out, strlen(buffer_out));
    pthread_mutex_unlock(&game_state->mutex);
    return NULL;
}

void *player_turn(void *arg) {
    GameState *game_state = (GameState *)arg;
    char buffer_in[1024];
    char buffer_out[1024];
    int bytes_read;

    while (1) {
        pthread_mutex_lock(&game_state->mutex);
        while (game_state->player_turn == 0) {
            pthread_cond_wait(&game_state->cond_player, &game_state->mutex);
        }
        pthread_mutex_unlock(&game_state->mutex);
        print_out_dealer_first(game_state->dealer, game_state->client_socket);
        print_out_player_cards(game_state->player, game_state->client_socket);
        snprintf(buffer_out, sizeof(buffer_out), "Enter 'hit' to draw a card, 'stand' to end your turn: ");
        write(game_state->client_socket, buffer_out, strlen(buffer_out));
        bytes_read = read(game_state->client_socket, buffer_in, sizeof(buffer_in) - 1);
        if (bytes_read <= 0) {
            snprintf(buffer_out, sizeof(buffer_out), "Error reading input.\n");
            write(game_state->client_socket, buffer_out, strlen(buffer_out));
            break;
        }
        buffer_in[bytes_read] = '\0';
        pthread_mutex_lock(&game_state->mutex);

        if (strcmp(buffer_in, "hit") == 0) {
            game_state->player->cards[game_state->player->card_count++] = drawCardGame(game_state);
            int player_value = calculate_hand_value(game_state->player);
            if (player_value > 21) {
                print_out_player_cards(game_state->player, game_state->client_socket);
                game_state->player_turn = 0;
                pthread_cond_signal(&game_state->cond_dealer); 
                pthread_mutex_unlock(&game_state->mutex);
                break;
            }
            print_out_separators(game_state->client_socket);
            print_out_player_cards(game_state->player, game_state->client_socket);
        } else if (strcmp(buffer_in, "stand") == 0) {
            snprintf(buffer_out, sizeof(buffer_out), "You stand with %d cards.\n", game_state->player->card_count);
            write(game_state->client_socket, buffer_out, strlen(buffer_out));
            print_out_separators(game_state->client_socket);
            game_state->player_turn = 0;
            pthread_cond_signal(&game_state->cond_dealer); 
            
            pthread_mutex_unlock(&game_state->mutex);
            break;
        }
        pthread_mutex_unlock(&game_state->mutex);
    }

    return NULL;
}

void give_player_two_cards(Player* player, GameState* game_state) {
    player->cards[player->card_count++] = drawCardGame(game_state);
    player->cards[player->card_count++] = drawCardGame(game_state);
    player->card_sum = calculate_hand_value(player);
}

void remove_player_cards(Player *player) {
    for (int i = 0; i < player->card_count; i++) {
        player->cards[i] = NULL;
    }
    player->card_count = 0;
}


void start_blackjack_game(int client_socket, GameState *game_state, Player *player) {
    char buffer_in[1024];
    char buffer_out[1024];
    int bytes_read;
    game_state->player = player;
    pthread_t player_thread, dealer_thread;
    
    initializeDeck();
    shuffleDeck();
    
    while (1) {
        
        snprintf(buffer_out, sizeof(buffer_out), "Your balance: %d \nIf you want to exit type 'exit'\nEnter amount that you would like to bet: ", player->chips);
        write(client_socket, buffer_out, strlen(buffer_out));
        bytes_read = read(game_state->client_socket, buffer_in, sizeof(buffer_in) - 1);
        buffer_in[bytes_read] = '\0';
        if (bytes_read <= 0) {
            snprintf(buffer_out, sizeof(buffer_out), "Error reading input.\n");
            write(client_socket, buffer_out, strlen(buffer_out));
            continue;
        }

        int bet = atoi(buffer_in);
        if (bet > player->chips) {
            snprintf(buffer_out, sizeof(buffer_out), "You don't have enough balance to bet that amount.\n");
            write(client_socket, buffer_out, strlen(buffer_out));
            continue;
        }
        
        
        pthread_mutex_lock(&game_state->mutex);
        remove_player_cards(game_state->player);
        give_player_two_cards(game_state->player,game_state);
        player->chips -= bet;
        remove_player_cards(game_state->dealer);
        give_player_two_cards(game_state->dealer,game_state);
        game_state->dealer->card_sum = calculate_first_dealer_value(game_state->dealer);
        game_state->player_turn = 1;
        game_state->player_bet = bet;
        pthread_cond_signal(&game_state->cond_player);
        pthread_mutex_unlock(&game_state->mutex);
        if (pthread_create(&player_thread, NULL, player_turn, (void *)game_state) != 0) {
            perror("Failed to create player thread");
            return;
        }

        if (pthread_create(&dealer_thread, NULL, dealer_turn, (void *)game_state) != 0) {
            perror("Failed to create dealer thread");
            return;
        }
        pthread_join(player_thread, NULL);
        pthread_join(dealer_thread, NULL);
        if (game_state->game_over == 1) {
            cleanup_game(game_state, game_state->shm_id);
            break;
        }
    }
    
}
