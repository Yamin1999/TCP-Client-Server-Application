#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_LINE_LENGTH 1024
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

typedef struct {
    int user_id;
    char first_name[50];
    char last_name[50];
    char email[100];
    char city[50];
} User;

// Function to parse a CSV line and store it in a User struct
int parse_csv_line(const char *line, User *user) {
    return sscanf(line, "%d,%49[^,],%49[^,],%99[^,],%49[^\n]", 
                 &user->user_id, user->first_name, user->last_name, 
                 user->email, user->city);
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

    printf("CSV parsing functionality added\n");
    
    // Test CSV parsing (without server connection)
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
        printf("Skipping header line: %s", line);
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

        printf("Parsed user: %d, %s %s, %s, %s\n", 
               user.user_id, user.first_name, user.last_name, user.email, user.city);
        
        // Server communication will be implemented in future commits
    }

    // Clean up
    close(sock_fd);
    fclose(csv_file);
    fclose(log_file);
    
    return 0;
}