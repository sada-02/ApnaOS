#ifndef USERS_H
#define USERS_H

#include <stdint.h>
#include <stddef.h>

#define MAX_USERS 16
#define MAX_USERNAME_LEN 32
#define MAX_PASSWORD_LEN 64
#define PASSWORD_HASH_SIZE 32

typedef struct {
    char username[MAX_USERNAME_LEN];
    uint8_t password_hash[PASSWORD_HASH_SIZE];
    uint32_t home_dir_inode;
    int is_active;
} User;

void init_user_system();
int create_user(const char* username, const char* password);
int authenticate_user(const char* username, const char* password);
int switch_to_user(const char* username, const char* password);
void switch_to_root();
const char* get_current_username();
int is_root_user();
uint32_t get_current_user_home_inode();

#endif
