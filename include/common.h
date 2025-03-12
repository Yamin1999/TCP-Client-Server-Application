#ifndef COMMON_H
#define COMMON_H

#define SERVER_PORT 8080
#define SERVER_IP "127.0.0.1"
#define MAX_LINE_LENGTH 1024

// Shared data structure for user information
typedef struct {
    int user_id;
    char first_name[50];
    char last_name[50];
    char email[100];
    char city[50];
} User;

#endif /* COMMON_H */