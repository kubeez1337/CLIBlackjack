#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/shm.h>

void *attach_shared_memory(key_t key, size_t size) {
    int shm_id = shmget(key, size, 0666);
    if (shm_id == -1) {
        perror("shmget failed");
        exit(1);
    }

    void *shm_ptr = shmat(shm_id, NULL, 0);
    if (shm_ptr == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    return shm_ptr;
}

void start_client(const char *server_name, int port, Player *player) {
    char tmp_file[256];
    snprintf(tmp_file, sizeof(tmp_file), "/tmp/%s.tmp", server_name);

    key_t shm_key = ftok(tmp_file, 'S');
    if (shm_key == -1) {
        perror("ftok failed");
        exit(1);
    }

    GameState *game_state = (GameState *)attach_shared_memory(shm_key, sizeof(GameState));

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket failed");
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(client_socket);
        exit(1);
    }

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect failed");
        close(client_socket);
        exit(1);
    }
    char buffer_in[1024];
    char buffer_out[1024];
    printf("Connected to server '%s'\n", server_name);

    write(client_socket, player, sizeof(Player));
    //game_state->player = player;
    // Listen to the server
    fd_set read_fds;
    
    int bytes_read;
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(client_socket, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        int max_fd = client_socket > STDIN_FILENO ? client_socket : STDIN_FILENO;

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select failed");
            break;
        }

        if (FD_ISSET(client_socket, &read_fds)) {
            bytes_read = read(client_socket, buffer_in, sizeof(buffer_in) - 1);
            if (bytes_read > 0) {
                buffer_in[bytes_read] = '\0';
                printf("%s\n", buffer_in);
            } else {
                break;
            }
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            fgets(buffer_out, sizeof(buffer_out), stdin);
            buffer_out[strcspn(buffer_out, "\n")] = '\0';
            write(client_socket, buffer_out, strlen(buffer_out));
            if (strcmp(buffer_out, "exit") == 0) {
                write(client_socket, buffer_out, strlen(buffer_out));
                break;
            }
        }
    }

    close(client_socket);
    shmdt(game_state);
}