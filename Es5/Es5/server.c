#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include "list.h"

#define BUFFER_SIZE 2048
#define SERVER_PORT 8000
#define MAX_CLIENTS 10
 
volatile sig_atomic_t keep_running = 1;
 
void handle_signal(int sig) {
    printf("\nReceived signal %d. Shutting down server...\n", sig);
    keep_running = 0;
}
 
void process_message(char* message) {
    time_t current_time;
    time(&current_time);
     
    printf("[%s] Processing message: %s\n", ctime(&current_time), message);
     
    for (int i = 0; message[i]; i++) {
        message[i] = toupper(message[i]);
    }
}
 
void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
     
    memset(buffer, 0, BUFFER_SIZE);
     
    bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received < 0) {
        perror("Error receiving data from client");
        return;
    } else if (bytes_received == 0) {
        printf("Client disconnected\n");
        return;
    }
     
    buffer[bytes_received] = '\0';
    
    printf("Received message: \"%s\"\n", buffer);
     
    process_message(buffer);
     
    if (send(client_socket, buffer, strlen(buffer) + 1, 0) < 0) {
        perror("Error sending response to client");
    } else {
        printf("Response sent: \"%s\"\n", buffer);
    }
}
 
int parse_arguments(int argc, char* argv[], int* port) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) {
            if (i + 1 < argc) {
                *port = atoi(argv[i + 1]);
                if (*port <= 0 || *port > 65535) {
                    fprintf(stderr, "Invalid port number: %s\n", argv[i + 1]);
                    return 0;
                } 
            } else {
                fprintf(stderr, "Missing port number after %s\n", argv[i]);
                return 0;
            }
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            return 0;
        }
    }
    return 1;
}
 
int setup_server(int port) {
    int server_socket;
    struct sockaddr_in server_addr;
     
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }
     
    int option = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, 
                  &option, sizeof(option)) < 0) {
        perror("Setsockopt failed");
        close(server_socket);
        return -1;
    }
     
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
     
    if (bind(server_socket, (struct sockaddr *)&server_addr, 
            sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        return -1;
    }
     
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_socket);
        return -1;
    }
    
    return server_socket;
}

int main(int argc, char* argv[]) {
    int server_socket, client_socket;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int server_port = SERVER_PORT;
     
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
     
    signal(SIGCHLD, SIG_IGN);
     
    if (!parse_arguments(argc, argv, &server_port)) {
        printf("Usage: %s [-p|--port PORT]\n", argv[0]);
        return 1;
    }
     
    server_socket = setup_server(server_port);
    if (server_socket < 0) {
        return 1;
    }
    
    printf("Server started. Listening on port %d...\n", server_port);
     
    while (keep_running) {
        fd_set read_fds;
        struct timeval timeout;
         
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
         
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
         
        int activity = select(server_socket + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity < 0 && keep_running) {
            perror("Select error");
            continue;
        }
         
        if (activity == 0) {
            continue;
        }
         
        if (FD_ISSET(server_socket, &read_fds)) { 
            client_socket = accept(server_socket, 
                                 (struct sockaddr *)&client_addr, 
                                 &client_addr_len);
            
            if (client_socket < 0) {
                perror("Accept failed");
                continue;
            }
             
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
            printf("New connection from %s:%d\n", 
                   client_ip, ntohs(client_addr.sin_port));
             
            pid_t pid = fork();
            
            if (pid < 0) {
                perror("Fork failed");
                close(client_socket);
            } else if (pid == 0) { 
                close(server_socket);
                handle_client(client_socket);
                close(client_socket);
                exit(0);
            } else { 
                close(client_socket);
            }
        }
    }
     
    printf("Closing server socket...\n");
    close(server_socket);
    printf("Server shut down successfully\n");
    
    return 0;
}