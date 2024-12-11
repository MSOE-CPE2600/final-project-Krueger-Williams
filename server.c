#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully\n");

    // Bind to port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    printf("Socket options set successfully\n");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket bound successfully to port %d\n", PORT);

    // Listen
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Server is listening on port %d\n", PORT);

    while (1) {
        printf("Waiting for a connection...\n");

        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("Connection established\n");

        // Receive and send messages
        while (1) {
            int valread = read(new_socket, buffer, BUFFER_SIZE);
            if (valread <= 0) {
                printf("Connection closed by client\n");
                break;
            }
            printf("Received: %s\n", buffer);
            send(new_socket, buffer, strlen(buffer), 0);
            memset(buffer, 0, BUFFER_SIZE);
        }

        close(new_socket);
    }

    return 0;
}
