#include "server.h"
#include <signal.h> // Include for signal handling
#include <sys/wait.h> // Include for waitpid
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>


void *create_shared_memory(size_t size, key_t key, int *shm_id) {
    *shm_id = shmget(key, size, IPC_CREAT | 0666);
    if (*shm_id == -1) {
        perror("shmget failed");
        exit(1);
    }

    void *shm_ptr = shmat(*shm_id, NULL, 0);
    if (shm_ptr == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    return shm_ptr;
}

void start_server(const char *server_name, int port) {
    char tmp_file[256];
    snprintf(tmp_file, sizeof(tmp_file), "/tmp/%s.tmp", server_name);
    int fd = open(tmp_file, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("open failed");
        exit(1);
    }
    close(fd);

    key_t shm_key = ftok(tmp_file, 'S');
    if (shm_key == -1) {
        perror("ftok failed");
        unlink(tmp_file); 
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        unlink(tmp_file); 
        exit(1);
    }
    if (pid > 0) {
        printf("Server started with PID %d\n", pid);
        return;
    }

    setsid();

    int shm_id;
    GameState *game_state = (GameState *)create_shared_memory(sizeof(GameState), shm_key, &shm_id);
    game_state->client_count = 0;

    game_state->player = (Player *)malloc(sizeof(Player));
    game_state->dealer = (Player *)malloc(sizeof(Player));

    game_state->dealer->chips = 0;
    game_state->dealer->card_count = 0;
    strncpy(game_state->dealer->name, "Dealer", sizeof(game_state->dealer->name) - 1);
    game_state->dealer->name[sizeof(game_state->dealer->name) - 1] = '\0';
    game_state->shm_id = shm_id;

    if (pthread_mutex_init(&game_state->mutex, NULL) != 0) {
        perror("mutex init failed");
        exit(1);
    }
    if (pthread_cond_init(&game_state->cond_player, NULL) != 0) {
        perror("cond_player init failed");
        exit(1);
    }
    if (pthread_cond_init(&game_state->cond_dealer, NULL) != 0) {
        perror("cond_dealer init failed");
        exit(1);
    }

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket failed");
        exit(1);
    }

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt failed");
        close(server_socket);
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind failed");
        close(server_socket);
        exit(1);
    }

    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("listen failed");
        close(server_socket);
        exit(1);
    }

    printf("Server '%s' is running on port %d\n", server_name, port);

    while (1) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == -1) {
            perror("accept failed");
            continue;
        }

        if (fork() == 0) {
            close(server_socket);
            Player player;
            read(client_socket, &player, sizeof(Player));
            handle_client(client_socket, server_socket, shm_id, &player, tmp_file, game_state);
            exit(0);
        }
        close(client_socket);
    }

    //cleanup_game(game_state, shm_id);
    close(server_socket);
    unlink(tmp_file);
    exit(0);
}

void handle_client(int client_socket, int server_socket, int shm_id, Player *player, const char *tmp_file, GameState *game_state) {
    char buffer_in[1024];
    char buffer_out[1024];
    int bytes_read;

    if (player->chips == 0) {
        player->chips = 100;
    }
    game_state->client_socket = client_socket;
    // Deck* deck;
    // game_state->deck = deck;
    start_blackjack_game(client_socket, game_state, player);

    while ((bytes_read = read(client_socket, buffer_in, sizeof(buffer_in) - 1)) > 0) {
        buffer_in[bytes_read] = '\0';
        printf("Received: %s\n", buffer_in);

        if (strcmp(buffer_in, "exit") == 0) {
            printf("Shutting down server...\n");
            close(client_socket);
            close(server_socket);
            //cleanup_game(game_state, shm_id);
            unlink(tmp_file);
            exit(0);
        }
    }

    close(client_socket);
}

void cleanup_game(GameState *game_state, int shm_id) {
    cleanCards();
    if (game_state->player != NULL) {
        free(game_state->player);
        game_state->player = NULL;
    }
    if (game_state->dealer != NULL) {
        free(game_state->dealer);
        game_state->dealer = NULL;
    }

    pthread_cond_destroy(&game_state->cond_player);
    pthread_cond_destroy(&game_state->cond_dealer);
    pthread_mutex_destroy(&game_state->mutex);

    shmdt(game_state);
    shmctl(shm_id, IPC_RMID, NULL);
    
}