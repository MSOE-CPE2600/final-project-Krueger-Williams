#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define PROXY_HOST "maccancode.com"  // Replace with your proxy server's IP address
#define PROXY_PORT 9091  // Port for server connections

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    struct hostent *host_entry;

    // Get the proxy server address
    host_entry = gethostbyname(PROXY_HOST);
    if (host_entry == NULL) {
        perror("gethostbyname failed");
        exit(EXIT_FAILURE);
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }
    printf("Socket created successfully\n");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PROXY_PORT);
    serv_addr.sin_addr = *((struct in_addr*) host_entry->h_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }
    printf("Connected to the proxy server\n");

    while (1) {
        // Send a message to the proxy server
        strcpy(buffer, "Hello from server");
        if (send(sock, buffer, strlen(buffer), 0) == -1) {
            perror("send failed");
            break;
        }
        memset(buffer, 0, BUFFER_SIZE);

        // Receive a message from the proxy server
        int valread = read(sock, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            if (valread == 0) {
                printf("Connection closed by proxy server\n");
            } else {
                perror("read failed");
            }
            break;
        }
        printf("Received from proxy: %s\n", buffer);
    }

    close(sock);
    return 0;
}
