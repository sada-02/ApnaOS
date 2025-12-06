#include "users.h"
#include "../keyboard/string.h"

extern void debug_print(const char* msg);
extern void print_to_screen(const char* message);
extern int create_user_space(int user_id);
extern void set_current_user_space(int user_id);
extern void switch_to_root_space();
extern void get_current_path(char* buffer, size_t size);
extern void switch_history_context(int user_id);

static User users[MAX_USERS];
static int user_count = 0;
static int current_user_index = -1;

uint32_t simple_hash(const char* str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

void hash_password(const char* password, uint8_t* hash) {
    uint32_t h = simple_hash(password);
    for (int i = 0; i < PASSWORD_HASH_SIZE; i++) {
        hash[i] = (uint8_t)((h >> (i % 4 * 8)) & 0xFF);
        h = simple_hash((const char*)&h);
    }
}

int compare_hash(const uint8_t* hash1, const uint8_t* hash2) {
    for (int i = 0; i < PASSWORD_HASH_SIZE; i++) {
        if (hash1[i] != hash2[i]) {
            return 0;
        }
    }
    return 1;
}

void init_user_system() {
    for (int i = 0; i < MAX_USERS; i++) {
        users[i].is_active = 0;
        users[i].home_dir_inode = 0;
    }
    
    strncpy(users[0].username, "root", MAX_USERNAME_LEN);
    users[0].home_dir_inode = 1;
    users[0].is_active = 1;
    user_count = 1;
    current_user_index = 0;
    
    debug_print("DEBUG: User system initialized with root user.");
}

int find_user(const char* username) {
    for (int i = 0; i < user_count; i++) {
        if (users[i].is_active && strcmp(users[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

int create_user(const char* username, const char* password) {
    if (user_count >= MAX_USERS) {
        debug_print("ERROR: Maximum users reached.");
        return -1;
    }
    
    if (find_user(username) != -1) {
        debug_print("ERROR: User already exists.");
        return -1;
    }
    
    if (strlen(username) == 0 || strlen(username) >= MAX_USERNAME_LEN) {
        debug_print("ERROR: Invalid username length.");
        return -1;
    }
    
    if (strlen(password) == 0 || strlen(password) >= MAX_PASSWORD_LEN) {
        debug_print("ERROR: Invalid password length.");
        return -1;
    }
    
    int user_idx = user_count;
    strncpy(users[user_idx].username, username, MAX_USERNAME_LEN);
    hash_password(password, users[user_idx].password_hash);
    users[user_idx].is_active = 1;
    
    int inode_idx = create_user_space(user_idx);
    if (inode_idx == -1) {
        users[user_idx].is_active = 0;
        debug_print("ERROR: Failed to create user space.");
        return -1;
    }
    
    users[user_idx].home_dir_inode = inode_idx + 1;
    user_count++;
    debug_print("DEBUG: User created successfully.");
    return user_idx;
}

int authenticate_user(const char* username, const char* password) {
    int user_idx = find_user(username);
    if (user_idx == -1) {
        return 0;
    }
    
    uint8_t input_hash[PASSWORD_HASH_SIZE];
    hash_password(password, input_hash);
    
    return compare_hash(users[user_idx].password_hash, input_hash);
}

int switch_to_user(const char* username, const char* password) {
    if (strcmp(username, "root") == 0) {
        switch_to_root();
        return 0;
    }
    
    int user_idx = find_user(username);
    if (user_idx == -1) {
        debug_print("ERROR: User not found.");
        return -1;
    }
    
    if (!authenticate_user(username, password)) {
        debug_print("ERROR: Authentication failed.");
        return -1;
    }
    
    current_user_index = user_idx;
    set_current_user_space(user_idx);
    switch_history_context(user_idx);
    
    debug_print("DEBUG: Switched to user.");
    return 0;
}

void switch_to_root() {
    current_user_index = 0;
    switch_to_root_space();
    switch_history_context(0);
    debug_print("DEBUG: Switched to root.");
}

const char* get_current_username() {
    if (current_user_index >= 0 && current_user_index < user_count) {
        return users[current_user_index].username;
    }
    return "unknown";
}

int is_root_user() {
    return (current_user_index == 0);
}

uint32_t get_current_user_home_inode() {
    if (current_user_index >= 0 && current_user_index < user_count) {
        return users[current_user_index].home_dir_inode;
    }
    return 1;
}
