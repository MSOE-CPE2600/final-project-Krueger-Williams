#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define HOSTNAME "turners.maccancode.com"  // Address of the Node.js proxy
#define PORT 9090  // Port of the Node.js proxy
#define PREFIX ": "

static int sock = 0; // Global Socket
static char username[100]; //Global Username

void* readingThread(void* sock);
void* writingThread(void* sock);

void handle_sigint() {
    close(sock);
    printf("\nProgram Ended.\n");
    exit(0);
}

int main() {

    printf("Please Enter a Username: \n");
    scanf("%s", username);

    if (signal(SIGINT, handle_sigint) == SIG_ERR) { 
        perror("signal"); exit(1); 
    }

    int sock = 0;
    struct sockaddr_in serv_addr;
    struct hostent *host_entry;

    // Get the server address
    host_entry = gethostbyname(HOSTNAME);
    if (host_entry == NULL) {
        perror("gethostbyname");
        exit(1);
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr = *((struct in_addr*) host_entry->h_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("Connected to the server\n");

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

/*void* writingThread(void* sock_ptr) {
    int sock = *((int*)sock_ptr);
    char buffer[BUFFER_SIZE] = {0};
    
    while (1) {
        // Get user input or any data to send
        printf("Enter message: ");
        fgets(buffer, BUFFER_SIZE, stdin);

        // Add the prefix to the start of the buffer 
        snprintf(buffer, BUFFER_SIZE, "%s%s", username, PREFIX);
        
        // Send a message to the proxy server
        if (send(sock, buffer, strlen(buffer), 0) == -1) {
            perror("send failed");
            break;
        }
        memset(buffer, 0, BUFFER_SIZE);
    }
    return NULL;
}*/

void* writingThread(void* sock_ptr) {
    int sock = *((int*)sock_ptr);
    char buffer[BUFFER_SIZE] = {0};
    char temp[BUFFER_SIZE] = {0};  // Temporary buffer to hold user input

    while (1) {
        // Get user input or any data to send
        printf("Enter message: ");
        fgets(temp, BUFFER_SIZE, stdin);

        // Remove the newline character at the end of temp
        size_t len = strlen(temp);
        if (len > 0 && temp[len - 1] == '\n') {
            temp[len - 1] = '\0';
        }

        // Add the username and prefix to the start of the buffer
        snprintf(buffer, BUFFER_SIZE, "%s%s%s", username, PREFIX, temp);

        // Send a message to the proxy server
        if (send(sock, buffer, strlen(buffer), 0) == -1) {
            perror("send failed");
            break;
        }
        memset(buffer, 0, BUFFER_SIZE);
        memset(temp, 0, BUFFER_SIZE);
    }
    return NULL;
}
