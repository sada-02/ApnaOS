#include "string.h"

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

void* memcpy(void* dest, const void* src, size_t n) {
    char* d = dest;
    const char* s = src;
    while (n--) *d++ = *s++;
    return dest;
}

void* memset(void* s, int c, size_t n) {
    unsigned char* p = s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n-- && *s1 && *s2) {
        if (*s1 != *s2) return (unsigned char)*s1 - (unsigned char)*s2;
        s1++;
        s2++;
    }
    return n == (size_t)-1 ? 0 : (unsigned char)*s1 - (unsigned char)*s2;
}

char* strncpy(char* dest, const char* src, size_t n) {
    char* d = dest;
    while (n-- && (*d++ = *src++));
    while (n-- > 0) *d++ = '\0';
    return dest;
}

char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == (char)c)
            return (char*) s;
        s++;
    }
    return 0;
}

char *strtok(char *str, const char *delim) {
    static char *static_str = 0;
    if (str) {
        static_str = str;
    }
    if (!static_str) {
        return 0;
    }
    while (*static_str && strchr(delim, *static_str)) {
        static_str++;
    }
    if (*static_str == '\0') {
        return 0;
    }
    char *token = static_str;
    while (*static_str && !strchr(delim, *static_str)) {
        static_str++;
    }
    if (*static_str) {
        *static_str = '\0';
        static_str++;
    }
    return token;
}
