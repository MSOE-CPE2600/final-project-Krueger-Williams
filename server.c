#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define PROXY_HOST "maccancode.com"  // Replace with your proxy server's IP address
#define PROXY_PORT 9091  // Port for server connections

static int sock = 0; // Global Socket

void* readingThread(void* sock);
void* writingThread(void* sock);

void handle_sigint() {
    close(sock);
    printf("\nProgram Ended.\n");
}

int main() {
    if (signal(SIGINT, handle_sigint) == SIG_ERR) { 
        perror("signal"); exit(1); 
    }
    struct sockaddr_in serv_addr;
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

    // Kickoff Threads
    pthread_t read_thread;
    pthread_t write_thread;
    
    int result = pthread_create(&read_thread, NULL, readingThread, &sock);
    if (result != 0) {
        printf("Unable to create read thread\n");
        return -1;
    }

    result = pthread_create(&write_thread, NULL, writingThread, &sock);
    if (result != 0) {
        printf("Unable to create write thread\n");
        return -1;
    }
    
    // Wait for threads to complete
    pthread_join(read_thread, NULL);
    pthread_join(write_thread, NULL);
    
    close(sock);
    return 0;
}

void* readingThread(void* sock_ptr) {
    int sock = *((int*)sock_ptr);
    char buffer[BUFFER_SIZE] = {0};
    while (1) {
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
    return NULL;
}

void* writingThread(void* sock_ptr) {
    int sock = *((int*)sock_ptr);
    char buffer[BUFFER_SIZE] = {0};
    while (1) {
        // Get user input or any data to send
        printf("Enter message: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        
        // Send a message to the proxy server
        if (send(sock, buffer, strlen(buffer), 0) == -1) {
            perror("send failed");
            break;
        }
        memset(buffer, 0, BUFFER_SIZE);
    }
    return NULL;
}
