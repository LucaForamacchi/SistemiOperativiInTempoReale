#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 2048
#define HUB_HOST "127.0.0.1"
#define HUB_PORT 8080
#define MAX_NAME_LENGTH 50
#define MIN_TEMP -10.0
#define MAX_TEMP 40.0
#define MIN_DELAY 1
#define MAX_DELAY 8

typedef struct {
    char name[MAX_NAME_LENGTH];
    float temperature;
} SensorReading;


float random_float(float min, float max) {
    float scale = rand() / (float) RAND_MAX;
    return min + scale * (max - min);
}


int send_sensor_data(const char* sensor_name) {
    int sockfd;
    struct sockaddr_in hub_addr;
    struct hostent* hub;
    char buffer[BUFFER_SIZE];
    SensorReading reading;
    
    strncpy(reading.name, sensor_name, MAX_NAME_LENGTH - 1);
    reading.temperature = random_float(MIN_TEMP, MAX_TEMP);
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }
    
    if ((hub = gethostbyname(HUB_HOST)) == NULL) {
        perror("Failed to resolve hub hostname");
        close(sockfd);
        return -1;
    }
    
    memset(&hub_addr, 0, sizeof(hub_addr));
    hub_addr.sin_family = AF_INET;
    hub_addr.sin_port = htons(HUB_PORT);
    memcpy(&hub_addr.sin_addr.s_addr, hub->h_addr, hub->h_length);
    
    if (connect(sockfd, (struct sockaddr *)&hub_addr, sizeof(hub_addr)) < 0) {
        perror("Connection to hub failed");
        close(sockfd);
        return -1;
    }

    snprintf(buffer, BUFFER_SIZE, "%s %.2f", reading.name, reading.temperature);
    
    int delay = MIN_DELAY + rand() % (MAX_DELAY - MIN_DELAY + 1);
    sleep(delay);
    
    printf("[PID %d] Sensor '%s' sending temperature: %.2f°C (delay: %d seconds)\n", 
           getpid(), reading.name, reading.temperature, delay);
    
    if (send(sockfd, buffer, strlen(buffer) + 1, 0) < 0) {
        perror("Failed to send data to hub");
        close(sockfd);
        return -1;
    }
    
    memset(buffer, 0, BUFFER_SIZE);
    if (recv(sockfd, buffer, BUFFER_SIZE, 0) < 0) {
        perror("Failed to receive hub response");
        close(sockfd);
        return -1;
    }
    
    printf("[PID %d] Hub response: \"%s\"\n", getpid(), buffer);
    
    close(sockfd);
    return 0;
}

void print_usage(const char* program_name) {
    printf("Usage: %s <sensor_name> <num_readings>\n", program_name);
    printf("  sensor_name: Name identifier for the sensor\n");
    printf("  num_readings: Number of temperature readings to send\n");
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }
    
    char sensor_name[MAX_NAME_LENGTH];
    int num_readings;
    
    strncpy(sensor_name, argv[1], MAX_NAME_LENGTH - 1);
    sensor_name[MAX_NAME_LENGTH - 1] = '\0';
    
    num_readings = atoi(argv[2]);
    if (num_readings <= 0) {
        printf("Error: Number of readings must be positive\n");
        return 1;
    }
    
    printf("Starting sensor '%s' with %d readings\n", sensor_name, num_readings);
    
    srand(time(NULL));
    
    pid_t child_pids[num_readings];
    for (int i = 0; i < num_readings; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("Fork failed");
            
            for (int j = 0; j < i; j++) {
                if (child_pids[j] > 0) {
                    kill(child_pids[j], SIGTERM);
                }
            }
            
            return 1;
        } else if (pid == 0) {
            char child_sensor_name[MAX_NAME_LENGTH];
            snprintf(child_sensor_name, MAX_NAME_LENGTH, "%s_%d", sensor_name, i+1);
            
            srand(time(NULL) ^ getpid());
            
            send_sensor_data(child_sensor_name);
            exit(0);
        } else {
            child_pids[i] = pid;
            printf("Created child process %d for reading #%d\n", pid, i+1);
        }
    }
    
    for (int i = 0; i < num_readings; i++) {
        int status;
        pid_t child_pid = wait(&status);
        printf("Child process %d completed with status %d\n", child_pid, 
               WIFEXITED(status) ? WEXITSTATUS(status) : -1);
    }
    
    printf("All sensor readings completed\n");
    return 0;
}