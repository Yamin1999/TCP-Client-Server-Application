#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <mysql/mysql.h>

#define SERVER_PORT 8080
#define MAX_PENDING 10

typedef struct {
    int user_id;
    char first_name[50];
    char last_name[50];
    char email[100];
    char city[50];
} User;

// MySQL Database connection info
#define DB_HOST "localhost"
#define DB_USER "root"       // Change as needed
#define DB_PASS "new_password"   // Change as needed
#define DB_NAME "user_db"    // Change as needed

// Global variables for cleanup
MYSQL *conn = NULL;
int server_fd = -1;
int running = 1;

// Signal handler for graceful shutdown
void handle_signal(int sig) {
    printf("Received signal %d, shutting down...\n", sig);
    running = 0;
}

// Initialize database connection
MYSQL* init_database() {
    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return NULL;
    }

    if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, 
                          DB_NAME, 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    // Create table if it doesn't exist
    const char *create_table_query = 
        "CREATE TABLE IF NOT EXISTS users ("
        "user_id INT PRIMARY KEY, "
        "first_name VARCHAR(50), "
        "last_name VARCHAR(50), "
        "email VARCHAR(100), "
        "city VARCHAR(50))";

    if (mysql_query(conn, create_table_query)) {
        fprintf(stderr, "Table creation failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    printf("Connected to MySQL database successfully\n");
    return conn;
}

// Initialize socket and start listening
int init_server() {
    int server_fd;
    struct sockaddr_in address;
    
    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }
    
    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_fd);
        return -1;
    }
    
    // Configure address
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SERVER_PORT);
    
    // Bind socket to address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return -1;
    }
    
    // Start listening
    if (listen(server_fd, MAX_PENDING) < 0) {
        perror("Listen failed");
        close(server_fd);
        return -1;
    }
    
    printf("Server listening on port %d\n", SERVER_PORT);
    return server_fd;
}

int main() {
    // Set up signal handlers
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    // Initialize database
    conn = init_database();
    if (conn == NULL) {
        return 1;
    }
    
    // Initialize server
    server_fd = init_server();
    if (server_fd < 0) {
        mysql_close(conn);
        return 1;
    }
    
    printf("Server initialized successfully with MySQL connectivity\n");
    printf("User data storage will be implemented in future commits\n");
    
    // Basic server loop
    while (running) {
        printf("Server running... Press Ctrl+C to exit\n");
        sleep(5);  // Just to prevent high CPU usage in this skeleton code
    }
    
    // Cleanup
    if (server_fd >= 0) {
        close(server_fd);
    }
    
    if (conn != NULL) {
        mysql_close(conn);
    }
    
    printf("Server shut down gracefully\n");
    return 0;
}