#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include "verbose.h"

// Глобальные переменные
extern int recursive_flag;
extern unsigned long long min_size;  // Минимальный размер файлов
extern unsigned long long max_size;  // Максимальный размер файлов

// Структура для хранения информации о файле
typedef struct {
    char full_path[PATH_MAX];
    off_t file_size;
} file_entry;

// Глобальные переменные для хранения списка файлов
file_entry *file_list = NULL;
size_t file_count = 0;
size_t file_list_capacity = 0;

// Прототипы функций данного модуля
void add_file_entry(const char *full_path, off_t file_size);
void scan_directory(const char *path);
void filter_size_list(void);  // Фильтрация по размеру
void cleanup_selection(void); // Функция для очистки ресурсов

// Добавление файла в глобальный список
void add_file_entry(const char *full_path, off_t file_size) {
    if (file_count >= file_list_capacity) {
        file_list_capacity = file_list_capacity ? file_list_capacity * 2 : 10;
        file_entry *tmp = realloc(file_list, file_list_capacity * sizeof(file_entry));
        if (!tmp) {
            perror("Ошибка выделения памяти");
            free(file_list);
            exit(EXIT_FAILURE);
        }
        file_list = tmp;
    }
    // Безопасное копирование строки с гарантированной нулевой терминаторой.
    snprintf(file_list[file_count].full_path, PATH_MAX, "%s", full_path);
    file_list[file_count].file_size = file_size;
    file_count++;
}

// Сканирование директории с возможностью рекурсии
void scan_directory(const char *path) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(path))) {
        perror("Ошибка открытия директории");
        return;
    }

    verbose_log("Сканирование директории:");
    verbose_log_path(path);

    while ((entry = readdir(dir)) != NULL) {
        // Игнорирование "." и ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat statbuf;
        if (stat(full_path, &statbuf) < 0) {
            perror("Ошибка получения информации");
            printf("Для: %s\n", full_path);
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            // Рекурсивный обход вложенных директорий
            if (recursive_flag) {
                verbose_log("Найден каталог:");
                verbose_log_path(full_path);
                scan_directory(full_path);
            }
        } else {
            add_file_entry(full_path, statbuf.st_size);
        }
    }

    closedir(dir);
}

// Фильтрация списка файлов по размеру
void filter_size_list(void) {
    if (min_size == 0 && max_size == 0)
        return;  // Фильтрация не требуется

    verbose_log("Ограничение поиска:");
    if (min_size > 0) {
        char log_msg[128];
        snprintf(log_msg, sizeof(log_msg), "Минимальный размер: %llu байт", min_size);
        verbose_log(log_msg);
    }
    if (max_size > 0) {
        char log_msg[128];
        snprintf(log_msg, sizeof(log_msg), "Максимальный размер: %llu байт", max_size);
        verbose_log(log_msg);
    } 

    size_t j = 0;
    for (size_t i = 0; i < file_count; i++) {
        off_t fsize = file_list[i].file_size;
        if ((min_size && (unsigned long long)fsize < min_size) ||
            (max_size && (unsigned long long)fsize > max_size))
            continue;  // Исключаем файлы, не соответствующие критериям

        file_list[j++] = file_list[i];  // Сохранение подходящих файлов
    }
    file_count = j;
}

// Функция для очистки выделенной памяти
void cleanup_selection(void) {
    if (file_list != NULL) {
        free(file_list);
        file_list = NULL;
        file_count = 0;
        file_list_capacity = 0;
    }
}