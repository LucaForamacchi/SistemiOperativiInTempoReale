#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 2048
#define MAX_CLIENTS 50
#define HUB_PORT 8080
#define SERVER_PORT 8000
#define SERVER_HOST "127.0.0.1"


char message_buffer[MAX_CLIENTS][BUFFER_SIZE];
int message_count = 0;
int threshold = 0;

void signal_handler(int sig) {
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0);
}

int forward_messages_to_server() {
    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent* server;
    char response[BUFFER_SIZE];
     
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }
     
    if ((server = gethostbyname(SERVER_HOST)) == NULL) {
        perror("Unable to resolve server hostname");
        close(sockfd);
        return -1;
    }
     
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
     
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(sockfd);
        return -1;
    }
    
    printf("Connected to central server. Forwarding %d messages...\n", threshold);
     
    for (int i = 0; i < threshold; i++) {
        printf("Forwarding message: \"%s\"\n", message_buffer[i]);
        
        if (send(sockfd, message_buffer[i], strlen(message_buffer[i]) + 1, 0) < 0) {
            perror("Failed to send message to server");
            close(sockfd);
            return -1;
        }
         
        memset(response, 0, BUFFER_SIZE);
        if (recv(sockfd, response, BUFFER_SIZE, 0) < 0) {
            perror("Failed to receive server response");
            
        } else {
            printf("Server response for message %d: \"%s\"\n", i+1, response);
        }
    }
    
    printf("All messages forwarded successfully.\n");
    close(sockfd);
    return 0;
}
 
void handle_client(int client_sockfd) {
    char buffer[BUFFER_SIZE];
     
    memset(buffer, 0, BUFFER_SIZE);
     
    if (recv(client_sockfd, buffer, BUFFER_SIZE, 0) < 0) {
        perror("Error receiving data from client");
        return;
    }
    
    printf("Received message from client: \"%s\"\n", buffer);
     
    if (message_count < MAX_CLIENTS) {
        strncpy(message_buffer[message_count], buffer, BUFFER_SIZE - 1);
        message_count++;
        
        printf("Message stored (%d/%d)\n", message_count, threshold);
         
        if (message_count >= threshold) {
            printf("Threshold reached. Forwarding messages to central server...\n");
            forward_messages_to_server();
             
            message_count = 0;
        }
    } else {
        printf("Warning: Message buffer is full, message discarded\n");
    }
     
    const char* ack = "Message received by hub";
    if (send(client_sockfd, ack, strlen(ack) + 1, 0) < 0) {
        perror("Error sending acknowledgment to client");
    }
}

int main(int argc, char *argv[]) {
    int server_sockfd, client_sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pid_t child_pid;
     
    if (argc < 2) {
        printf("Usage: %s <threshold>\n", argv[0]);
        printf("  threshold: Number of messages to collect before forwarding to server\n");
        return 1;
    }
     
    threshold = atoi(argv[1]);
    if (threshold <= 0 || threshold > MAX_CLIENTS) {
        printf("Error: Threshold must be between 1 and %d\n", MAX_CLIENTS);
        return 1;
    }
     
    memset(message_buffer, 0, sizeof(message_buffer));
     
    signal(SIGCHLD, signal_handler);
     
    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return 1;
    }
     
    int opt = 1;
    if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Setsockopt failed");
        close(server_sockfd);
        return 1;
    }
     
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(HUB_PORT);
     
    if (bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sockfd);
        return 1;
    }
     
    if (listen(server_sockfd, 10) < 0) {
        perror("Listen failed");
        close(server_sockfd);
        return 1;
    }
    
    printf("Hub started. Listening on port %d. Threshold set to %d messages.\n", 
           HUB_PORT, threshold);
     
    while (1) {
        printf("\n[PID %d] Waiting for incoming connections...\n", getpid());
         
        if ((client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, 
                                   &client_len)) < 0) {
            perror("Accept failed");
            continue;
        }
         
        if ((child_pid = fork()) < 0) {
            perror("Fork failed");
            close(client_sockfd);
            continue;
        }
        
        if (child_pid == 0) { 
            close(server_sockfd);
            printf("[PID %d] Child process handling client connection\n", getpid());
            handle_client(client_sockfd);
            close(client_sockfd);
            exit(0);
        } else { 
            printf("[PID %d] Created child process %d to handle client\n", getpid(), child_pid);
            close(client_sockfd);
        }
    }
    
    close(server_sockfd);
    return 0;
}