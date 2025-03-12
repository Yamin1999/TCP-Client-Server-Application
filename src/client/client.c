#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../../include/common.h"

// Function to parse a CSV line and store it in a User struct
int parse_csv_line(const char *line, User *user) {
    return sscanf(line, "%d,%49[^,],%49[^,],%99[^,],%49[^\n]", 
                 &user->user_id, user->first_name, user->last_name, 
                 user->email, user->city);
}

// Function to log the acknowledgment and user info
void log_acknowledgment(User *user, const char *ack, FILE *log_file) {
    fprintf(log_file, "User ID: %d, Name: %s %s, Email: %s, City: %s - Server Response: %s\n",
           user->user_id, user->first_name, user->last_name, user->email, user->city, ack);
    fflush(log_file);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <csv_file> <log_file>\n", argv[0]);
        return 1;
    }

    // Open CSV file
    FILE *csv_file = fopen(argv[1], "r");
    if (csv_file == NULL) {
        perror("Error opening CSV file");
        return 1;
    }

    // Open log file
    FILE *log_file = fopen(argv[2], "w");
    if (log_file == NULL) {
        perror("Error opening log file");
        fclose(csv_file);
        return 1;
    }

    // Create socket
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Error creating socket");
        fclose(csv_file);
        fclose(log_file);
        return 1;
    }

    // Configure server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock_fd);
        fclose(csv_file);
        fclose(log_file);
        return 1;
    }

    // Connect to server
    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock_fd);
        fclose(csv_file);
        fclose(log_file);
        return 1;
    }

    printf("Connected to server at %s:%d\n", SERVER_IP, SERVER_PORT);

    // Read CSV file line by line
    char line[MAX_LINE_LENGTH];
    // Skip header if present
    if (fgets(line, MAX_LINE_LENGTH, csv_file) == NULL) {
        printf("CSV file is empty\n");
        close(sock_fd);
        fclose(csv_file);
        fclose(log_file);
        return 1;
    }

    // Check if first line looks like header (does not start with a number)
    if (line[0] < '0' || line[0] > '9') {
        printf("Skipping header line\n");
    } else {
        // Reset file pointer to beginning if no header
        rewind(csv_file);
    }

    // Process each line
    while (fgets(line, MAX_LINE_LENGTH, csv_file) != NULL) {
        // Remove newline character if present
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }

        // Parse line
        User user;
        if (parse_csv_line(line, &user) != 5) {
            fprintf(stderr, "Error parsing line: %s\n", line);
            continue;
        }

        printf("Sending user: %d, %s %s, %s, %s\n", 
               user.user_id, user.first_name, user.last_name, user.email, user.city);

        // Send user data to server
        if (send(sock_fd, &user, sizeof(User), 0) < 0) {
            perror("Send failed");
            break;
        }

        // Receive acknowledgment
        char ack[256];
        ssize_t bytes_received = recv(sock_fd, ack, sizeof(ack) - 1, 0);
        if (bytes_received <= 0) {
            perror("Receive failed");
            break;
        }

        // Null-terminate the acknowledgment
        ack[bytes_received] = '\0';
        printf("Received acknowledgment: %s\n", ack);

        // Log the acknowledgment
        log_acknowledgment(&user, ack, log_file);

        // Small delay between requests
        usleep(100000);  // 100ms delay
    }

    // Clean up
    close(sock_fd);
    fclose(csv_file);
    fclose(log_file);
    printf("Client execution completed\n");

    return 0;
}