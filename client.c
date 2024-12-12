/********************************************************************
 *  Filename: client.c
 *  Class: CPE 2600 121
 *  Assignment Name: Lab 13 - Chat Application
 *  Description: Creates a TCP client connection for a messaging
 *      application running through a proxy server
 *  Author: Krueger 'Mac' Williams & Chance Mason
 *  Date:  12/11/2024
 *  Note: compile with make
 *          run with ./client
 *******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024 // Maximum chat message size
#define HOSTNAME "maccancode.com"  // Address of the Node.js proxy
#define PORT 9090  // Port of the Node.js proxy

static int sock = 0; // Globally Defined Socket
static char username[100]; // Global Username

void* readingThread(void* sock);
void* writingThread(void* sock);

// Closes program on SIGINT
void handle_sigint() {
    // Close connection
    close(sock);

    printf("\nChat Exited.\n");
    exit(0);
}

int main() {

    printf("Please Enter a Username: \n");

    // Get username
    fgets(username, 100, stdin);

    // Take \n off end of string
    size_t len = strlen(username); 
    if (len > 0 && username[len - 1] == '\n') { 
        // Replace last character (\n) with \0
        username[len - 1] = '\0'; 
    }

    // Register SIGNINT handler
    if (signal(SIGINT, handle_sigint) == SIG_ERR) { 
        perror("signal"); exit(1); 
    }

    // Get the server IP address from domain name
    struct hostent *host_entry;
    host_entry = gethostbyname(HOSTNAME);
    if (host_entry == NULL) {
        perror("gethostbyname");
        exit(1);
    }

    // Create a networking socket
    // AF_INET designates an external IPV4 address is being used
    // SOCK_STREAM designates a TCP Connection
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        exit(1);
    }


    struct sockaddr_in serv_addr;

    // Sets to IPV4 Address
    serv_addr.sin_family = AF_INET;

    // Sets the port number using htons() to ensure the correct byte order
    serv_addr.sin_port = htons(PORT);
    
    // Sets the IPV4 address obtained above
    serv_addr.sin_addr = *((struct in_addr*) host_entry->h_addr);

    // Creates the connection to the socket
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        exit(1);
    }

    printf("Connected to the server\n");

    pthread_t read_thread;
    pthread_t write_thread;
    
    // Launch thread to check for reads passing socket address
    int result = pthread_create(&read_thread, NULL, readingThread, &sock);
    if (result != 0) {
        printf("Unable to create read thread\n");
        return -1;
    }

    // Launch thread to check for writes passing socket address
    result = pthread_create(&write_thread, NULL, writingThread, &sock);
    if (result != 0) {
        printf("Unable to create write thread\n");
        return -1;
    }
    
    // Wait for threads to complete
    pthread_join(read_thread, NULL);
    pthread_join(write_thread, NULL);
    
    // Close socket connection
    close(sock);
    return 0;
}

void* readingThread(void* sock_ptr) {
    // Get socket from socket pointer
    int sock = *((int*)sock_ptr);

    // Setup buffer for message before printing
    char buffer[BUFFER_SIZE] = {0};

    // Keep checking for messages until user presses ctrl-c
    while (1) {
        // Clear buffer to 0s
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
        
        // Print message with formatting
        printf("\r\033[K%s\nEnter a message: ", buffer);
        fflush(stdout);
    }
    
    return NULL;
}

void* writingThread(void* sock_ptr) {
    // Get socket from socket pointer
    int sock = *((int*)sock_ptr);

    // Setup buffer for message before sending
    char buffer[BUFFER_SIZE] = {0};

    // Setup buffer to hold message before adding username
    char temp[BUFFER_SIZE] = {0};

    while (1) {

        printf("Enter message: ");

        // Get user message
        fgets(temp, BUFFER_SIZE, stdin);
        
        // Move terminal cursor up a line to overwrite the enter message prompt
        printf("\r\033[AO");
        
        // Remove the newline character at the end of temp
        size_t len = strlen(temp);
        if (len > 0 && temp[len - 1] == '\n') {
            temp[len - 1] = '\0';
        }

        // Add the username with a colon to front of message
        strncpy(buffer, username, BUFFER_SIZE); 
        strncat(buffer, ": ", BUFFER_SIZE - strlen(buffer) - 1); 
        strncat(buffer, temp, BUFFER_SIZE - strlen(buffer) - 1);

        // Send the message to the proxy server
        if (send(sock, buffer, strlen(buffer), 0) == -1) {
            perror("send failed");
            break;
        }

        // Empty message and temporary buffers
        memset(buffer, 0, BUFFER_SIZE);
        memset(temp, 0, BUFFER_SIZE);
    }

    return NULL;
}
