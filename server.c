/********************************************************************
 *  Filename: server.c
 *  Class: CPE 2600 121
 *  Assignment Name: Lab 13 - Chat Application
 *  Description: Creates a TCP server to manage clients in a 
 *      real-time messaging application
 *  Author: Krueger 'Mac' Williams
 *  Date:  12/11/2024
 *  Note:  compile with make
 *          run with ./server
 *******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024
#define PROXY_HOST "maccancode.com" // proxy server domain
#define PROXY_PORT 9091  // Port for proxy connections

static int proxy_sock = 0; // Global proxy socket

// Closes program on SIGINT
void handle_sigint() {
    // Close connection
    close(proxy_sock);

    printf("\nChat Exited.\n");
    exit(0);
}

void* clientHandler(void* client_sock_ptr);

int main() {

    // Setup signal handler for graceful exit
    if (signal(SIGINT, handle_sigint) == SIG_ERR) { 
        perror("signal"); 
        exit(1); 
    }

    // Get the server IP address from domain name
    struct hostent *host_entry;
    host_entry = gethostbyname(PROXY_HOST);
    if (host_entry == NULL) {
        perror("gethostbyname failed");
        exit(EXIT_FAILURE);
    }

    // Create a networking socket
    // AF_INET designates an external IPV4 address is being used
    // SOCK_STREAM designates a TCP Connection
    if ((proxy_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(1);
    }
    printf("Socket created successfully\n");

    struct sockaddr_in proxy_addr;

    // Sets to IPV4 Address
    proxy_addr.sin_family = AF_INET;

    // Sets the port number using htons() to ensure the correct byte order
    proxy_addr.sin_port = htons(PROXY_PORT);

    // Sets the IPV4 address obtained above
    proxy_addr.sin_addr = *((struct in_addr*) host_entry->h_addr);

    // Connect to the proxy server
    if (connect(proxy_sock, (struct sockaddr *)&proxy_addr, sizeof(proxy_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    printf("Connected to the proxy server\n");

    pthread_t client_thread;

    // Launch thread to manage clients
    int result = pthread_create(&client_thread, NULL, clientHandler, &proxy_sock);
    if (result != 0) {
        printf("Unable to create client handler thread\n");
        return -1;
    }

    // Wait for the client handler thread to complete
    pthread_join(client_thread, NULL);
    
    // Close socket
    close(proxy_sock);
    return 0;
}

void* clientHandler(void* client_sock_ptr) {
    // Get socket from socket pointer
    int proxy_sock = *((int*)client_sock_ptr);

    // Setup buffer for receiving and sending message
    char buffer[BUFFER_SIZE] = {0};

    while (1) {
        // Clear buffer to 0s
        memset(buffer, 0, BUFFER_SIZE);

        // Receive a message from the proxy server
        int valread = read(proxy_sock, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            if (valread == 0) {
                printf("Connection closed by proxy server\n");
            } else {
                perror("read failed");
            }
            break;
        }
        printf("Message from proxy: %s\n", buffer);
        
        // Take message and send out to all clients
        if (send(proxy_sock, buffer, strlen(buffer), 0) == -1) {
            perror("send failed");
            break;
        }
    }

    return NULL;
}
