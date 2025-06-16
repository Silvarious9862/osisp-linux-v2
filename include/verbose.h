#ifndef VERBOSE_H
#define VERBOSE_H

#include <stdio.h>

// ANSI escape-коды для цветов
#define BLUE   "\033[1;34m"  // Синий
#define GREEN  "\033[1;32m"  // Зеленый
#define RESET  "\033[0m"     // Сброс цвета

extern int verbose_flag;

// Логирование сообщений в режиме verbose
static void verbose_log(const char *message) {
    if (verbose_flag) {
        printf("[verbose] %s\n", message);
    }
}

#define MAX_VERBOSE_PATH_LENGTH 50  // Максимальная длина пути

// Логирование пути с обрезкой, если он слишком длинный
static void verbose_log_path(const char *full_path) {
    if (verbose_flag) {
        char truncated_path[MAX_VERBOSE_PATH_LENGTH + 1];
        size_t len = strlen(full_path);
        if (len > MAX_VERBOSE_PATH_LENGTH) {
            size_t tail_len = MAX_VERBOSE_PATH_LENGTH - 3; // Оставляем место для "..."
            snprintf(truncated_path, sizeof(truncated_path), "...%s", full_path + len - tail_len);
        } else {
            snprintf(truncated_path, sizeof(truncated_path), "%s", full_path);
        }
        printf("[verbose] %s\n", truncated_path);
    }
}

#endif // VERBOSE_H
