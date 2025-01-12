#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "server.h"
#include "client.h"
int main() {
    char user_name[256];
    char choice;
    char server_name[256];
    int server_port;
    Player player;
    player.chips = 0;
    printf("Enter your name: ");
    scanf("%s", user_name);
    player.card_count = 0;
    strncpy(player.name, user_name, sizeof(player.name) - 1);
    player.name[sizeof(player.name) - 1] = '\0';
    printf("Hello %s! Do you want to start a server or join an existing one?\n Type e to exit\n(s/j/e): ", user_name);
    scanf(" %c", &choice);
    while (choice != 'e'){
        if (choice == 's') {
            printf("Enter server name: ");
            scanf("%s", server_name);
            printf("Enter server port: ");
            scanf("%d", &server_port);

            pid_t pid = fork();
            if (pid < 0) {
                perror("fork failed");
                exit(1);
            }
            if (pid == 0) {
                start_server(server_name, server_port);
                exit(0);
            } else {
                sleep(2); 

                start_client(server_name, server_port, &player);
                
                waitpid(pid, NULL, 0);
            }
        } else if (choice == 'j') {
            printf("Enter server name to join: ");
            scanf("%s", server_name);
            printf("Enter server port: ");
            scanf("%d", &server_port);
            start_client(server_name, server_port, &player);
        } else {
            printf("Invalid choice\n");
        }

        printf("Hello %s! Do you want to start a server or join an existing one?\n Type e to exit\n(s/j/e): ", user_name);
        scanf(" %c", &choice);
    }

    return 0;
}