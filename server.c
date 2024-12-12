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
#define PROXY_HOST "maccancode.com"  // Replace with your proxy server's IP address
#define PROXY_PORT 9091  // Port for proxy connections

static int proxy_sock = 0; // Global proxy socket

void handle_sigint(int sig) {
    close(proxy_sock);
    printf("\nChat Exited.\n");
    exit(0); // Close program
}

void* clientHandler(void* client_sock_ptr);

int main() {
    struct sockaddr_in proxy_addr;
    struct hostent *host_entry;

    // Setup signal handler for graceful exit
    if (signal(SIGINT, handle_sigint) == SIG_ERR) { 
        perror("signal"); 
        exit(1); 
    }

    // Get the proxy server address
    host_entry = gethostbyname(PROXY_HOST);
    if (host_entry == NULL) {
        perror("gethostbyname failed");
        exit(EXIT_FAILURE);
    }

    // Create a socket for connecting to the proxy
    if ((proxy_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }
    printf("Socket created successfully\n");

    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_port = htons(PROXY_PORT);
    proxy_addr.sin_addr = *((struct in_addr*) host_entry->h_addr);

    // Connect to the proxy server
    if (connect(proxy_sock, (struct sockaddr *)&proxy_addr, sizeof(proxy_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }
    printf("Connected to the proxy server\n");

    pthread_t client_thread;
    int result = pthread_create(&client_thread, NULL, clientHandler, &proxy_sock);
    if (result != 0) {
        printf("Unable to create client handler thread\n");
        return -1;
    }

    // Wait for the client handler thread to complete
    pthread_join(client_thread, NULL);
    
    close(proxy_sock);
    return 0;
}

void* clientHandler(void* client_sock_ptr) {
    int proxy_sock = *((int*)client_sock_ptr);
    char buffer[BUFFER_SIZE] = {0};

    while (1) {
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
        
        // Process the message and send a response (echoing back in this example)
        if (send(proxy_sock, buffer, strlen(buffer), 0) == -1) {
            perror("send failed");
            break;
        }
    }

    return NULL;
}
