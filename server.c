#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mysql/mysql.h>
#include <signal.h>
#include <sys/select.h>
#include <errno.h>

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

// Store user in database
int store_user(MYSQL *conn, User *user) {
    char query[512];
    
    // Escape strings to prevent SQL injection
    char first_name_esc[100], last_name_esc[100], email_esc[200], city_esc[100];
    
    mysql_real_escape_string(conn, first_name_esc, user->first_name, strlen(user->first_name));
    mysql_real_escape_string(conn, last_name_esc, user->last_name, strlen(user->last_name));
    mysql_real_escape_string(conn, email_esc, user->email, strlen(user->email));
    mysql_real_escape_string(conn, city_esc, user->city, strlen(user->city));
    
    // Format query - using INSERT ... ON DUPLICATE KEY UPDATE for handling duplicates
    snprintf(query, sizeof(query), 
            "INSERT INTO users (user_id, first_name, last_name, email, city) "
            "VALUES (%d, '%s', '%s', '%s', '%s') "
            "ON DUPLICATE KEY UPDATE "
            "first_name='%s', last_name='%s', email='%s', city='%s'",
            user->user_id, first_name_esc, last_name_esc, email_esc, city_esc,
            first_name_esc, last_name_esc, email_esc, city_esc);
    
    // Execute query
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Insert failed: %s\n", mysql_error(conn));
        return 0;
    }
    
    return 1;
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

// Handle client connection
void handle_client(int client_fd, MYSQL *conn) {
    User user;
    
    while (running) {
        // Receive user data
        ssize_t bytes_received = recv(client_fd, &user, sizeof(User), 0);
        if (bytes_received <= 0) {
            // Client disconnected or error
            if (bytes_received < 0) {
                perror("Receive failed");
            }
            break;
        }
        
        printf("Received user: ID=%d, Name=%s %s, Email=%s, City=%s\n", 
               user.user_id, user.first_name, user.last_name, user.email, user.city);
        
        // Store user in database
        int success = store_user(conn, &user);
        
        // Prepare acknowledgment
        char ack[256];
        if (success) {
            snprintf(ack, sizeof(ack), "User ID %d processed successfully", user.user_id);
        } else {
            snprintf(ack, sizeof(ack), "Failed to process User ID %d", user.user_id);
        }
        
        // Send acknowledgment
        if (send(client_fd, ack, strlen(ack), 0) < 0) {
            perror("Send failed");
            break;
        }
    }
    
    close(client_fd);
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
    
    printf("Server initialized successfully\n");
    
    // Main server loop
    while (running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        // Accept client connection (with timeout to check running flag)
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        
        struct timeval timeout;
        timeout.tv_sec = 1;  // 1 second timeout
        timeout.tv_usec = 0;
        
        int activity = select(server_fd + 1, &readfds, NULL, NULL, &timeout);
        
        if (activity < 0 && errno != EINTR) {
            perror("Select error");
            break;
        }
        
        if (!running) break;
        
        if (activity > 0) {
            int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
            if (client_fd < 0) {
                perror("Accept failed");
                continue;
            }
            
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            printf("New connection from %s:%d\n", client_ip, ntohs(client_addr.sin_port));
            
            // Handle client in the same thread for simplicity
            // For production, you would want to use threads or a process pool
            handle_client(client_fd, conn);
        }
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