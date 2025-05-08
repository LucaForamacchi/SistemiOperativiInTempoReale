#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 2048
#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 8000

typedef struct {
    char operation[32];
    char identifier[32];
    float target_temp;
    char** sensor_list;
    int sensor_count;
} Request;

void construct_message(char* buffer, Request* req) {
    memset(buffer, 0, BUFFER_SIZE);
    
    strcat(buffer, req->operation);
    strcat(buffer, " ");
    strcat(buffer, req->identifier);
    
    if (strcmp(req->operation, "Iscrizione") == 0) {
        strcat(buffer, " ");
        for (int i = 0; i < req->sensor_count; i++) {
            strcat(buffer, req->sensor_list[i]);
            if (i < req->sensor_count - 1) {
                strcat(buffer, ";");
            }
        }
    }
}

void cleanup_resources(Request* req) {
    if (req->sensor_list) {
        for (int i = 0; i < req->sensor_count; i++) {
            free(req->sensor_list[i]);
        }
        free(req->sensor_list);
    }
}

void print_usage() {
    printf("Usage: ./attuatore <operation> <id> [target_temp] [sensor1 sensor2 ...]\n");
    printf("  operations: Iscrizione, Disiscrizione\n");
    printf("  For Iscrizione, provide target_temp and list of sensors\n");
    printf("  For Disiscrizione, only provide the id\n");
}

int connect_to_server(const char* host, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent* server;
    
    if ((server = gethostbyname(host)) == NULL) {
        perror("Error resolving hostname");
        return -1;
    }
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error creating socket");
        return -1;
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        close(sockfd);
        return -1;
    }
    
    return sockfd;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_usage();
        return 1;
    }
    
    Request req;
    memset(&req, 0, sizeof(Request));
     
    strncpy(req.operation, argv[1], sizeof(req.operation) - 1);
    strncpy(req.identifier, argv[2], sizeof(req.identifier) - 1);
    
    if (strcmp(req.operation, "Iscrizione") == 0) {
        if (argc < 4) {
            printf("Error: Iscrizione requires target temperature\n");
            print_usage();
            return 1;
        }
        
        req.target_temp = atof(argv[3]);
        req.sensor_count = argc - 4;
        
        if (req.sensor_count > 0) {
            req.sensor_list = malloc(req.sensor_count * sizeof(char*));
            if (!req.sensor_list) {
                perror("Memory allocation failed");
                return 1;
            }
            
            for (int i = 0; i < req.sensor_count; i++) {
                req.sensor_list[i] = strdup(argv[i + 4]);
                if (!req.sensor_list[i]) {
                    perror("Memory allocation failed");
                    cleanup_resources(&req);
                    return 1;
                }
            }
            
            printf("Sensors to register:\n");
            for (int i = 0; i < req.sensor_count; i++) {
                printf("- %s\n", req.sensor_list[i]);
            }
        }
    } else if (strcmp(req.operation, "Disiscrizione") == 0) {
        printf("Unregistering: %s\n", req.identifier);
    } else {
        printf("Error: Unknown operation '%s'\n", req.operation);
        print_usage();
        cleanup_resources(&req);
        return 1;
    }
    
    char buffer[BUFFER_SIZE];
    construct_message(buffer, &req);
    
    printf("Message to send: %s\n", buffer);
    
    int sockfd = connect_to_server(DEFAULT_HOST, DEFAULT_PORT);
    if (sockfd < 0) {
        cleanup_resources(&req);
        return 1;
    }
    
    printf("Sending request to server...\n");
    
    if (send(sockfd, buffer, strlen(buffer) + 1, 0) < 0) {
        perror("Error sending data to server");
        close(sockfd);
        cleanup_resources(&req);
        return 1;
    }
    
    printf("Request sent successfully. Waiting for response...\n");
    
    memset(buffer, 0, BUFFER_SIZE);
    if (recv(sockfd, buffer, BUFFER_SIZE, 0) < 0) {
        perror("Error receiving server response");
        close(sockfd);
        cleanup_resources(&req);
        return 1;
    }
    
    printf("Server response: \"%s\"\n", buffer);
    
    close(sockfd);
    cleanup_resources(&req);
    
    return 0;
}