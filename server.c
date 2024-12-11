#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>  // Include errno for error handling

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully\n");

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
        perror("setsockopt failed");
        close(server_fd);  // Close the socket on error
        exit(EXIT_FAILURE);
    }
    printf("Socket options set successfully\n");

    // Configure address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind to port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("bind failed");
        close(server_fd);  // Close the socket on error
        exit(EXIT_FAILURE);
    }
    printf("Socket bound successfully to port %d\n", PORT);

    // Listen for connections
    if (listen(server_fd, 3) == -1) {
        perror("listen failed");
        close(server_fd);  // Close the socket on error
        exit(EXIT_FAILURE);
    }
    printf("Server is listening on port %d\n", PORT);

    while (1) {
        printf("Waiting for a connection...\n");

        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket == -1) {
            perror("accept failed");
            close(server_fd);  // Optionally close the server socket
            exit(EXIT_FAILURE);
        }

        printf("Connection established with client\n");

        // Receive and send messages
        while (1) {
            int valread = read(new_socket, buffer, BUFFER_SIZE);
            if (valread <= 0) {
                if (valread == 0) {
                    printf("Connection closed by client\n");
                } else {
                    perror("read failed");
                }
                break;
            }
            printf("Received: %s\n", buffer);
            send(new_socket, buffer, strlen(buffer), 0);
            memset(buffer, 0, BUFFER_SIZE);
        }

        close(new_socket);
    }

    close(server_fd);  // Make sure to close the server socket when done
    return 0;
}
