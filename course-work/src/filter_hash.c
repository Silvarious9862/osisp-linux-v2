#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "verbose.h"
#include "md5.h"  // Наш собственный MD5

// Структура file_entry для хранения пути и размера файла
typedef struct {
    char full_path[PATH_MAX];
    off_t file_size;
} file_entry;

// Глобальные переменные
extern file_entry *file_list;
extern size_t file_count;
extern size_t file_list_capacity;

// Вспомогательная структура для хранения файла и его MD5-хеша
typedef struct {
    file_entry entry;
    char *hash;
} file_hash;

// Вычисление MD5-хеша файла с использованием нашей реализации MD5
static char *compute_md5(const char *filename) {
    char *md5_str = my_md5_file(filename);
    if (!md5_str) {
        // В случае ошибки возвращаем пустую строку
        md5_str = strdup("");
    }
    return md5_str;
}

// Сравнение MD5-хешей двух файлов
static int compare_file_hash(const void *a, const void *b) {
    return strcmp(((const file_hash *)a)->hash, ((const file_hash *)b)->hash);
}

// Фильтрация списка файлов по MD5-хешу
void filter_hash_list(void) {
    if (file_count == 0)
        return;

    file_hash *hash_array = malloc(file_count * sizeof(file_hash));
    if (!hash_array) {
        perror("Ошибка выделения памяти для hash_array");
        exit(EXIT_FAILURE);
    }

    // Вычисление MD5-хешей для всех файлов
    for (size_t i = 0; i < file_count; i++) {
        hash_array[i].entry = file_list[i];
        hash_array[i].hash = compute_md5(file_list[i].full_path);
        if (!hash_array[i].hash) {
            hash_array[i].hash = strdup("");
        }
    }

    // Сортировка файлов по MD5-хешу
    qsort(hash_array, file_count, sizeof(file_hash), compare_file_hash);

    file_entry *filtered_list = NULL;
    size_t filtered_count = 0, filtered_capacity = 0;
    int unique_flag = 0;
    verbose_log("Уникальные файлы:");

    size_t i = 0;
    while (i < file_count) {
        size_t j = i + 1;
        while (j < file_count && strcmp(hash_array[j].hash, hash_array[i].hash) == 0)
            j++;

        // Если MD5 совпадает у нескольких файлов, добавляем их в новый список
        if ((j - i) > 1) {
            for (size_t k = i; k < j; k++) {
                if (filtered_count >= filtered_capacity) {
                    filtered_capacity = filtered_capacity ? filtered_capacity * 2 : 10;
                    file_entry *temp = realloc(filtered_list, filtered_capacity * sizeof(file_entry));
                    if (!temp) {
                        perror("Ошибка выделения памяти для filtered_list");
                        // Освобождаем временные ресурсы при аварии:
                        for (size_t m = 0; m < file_count; m++)
                            free(hash_array[m].hash);
                        free(hash_array);
                        free(filtered_list);
                        free(file_list);
                        exit(EXIT_FAILURE);
                    }
                    filtered_list = temp;
                }
                filtered_list[filtered_count++] = hash_array[k].entry;
            }
        } else {
            verbose_log_path(file_list[i].full_path);
            unique_flag = 1;
        }
        i = j;
    }
    if (!unique_flag)
        verbose_log("Не найдены");

    // Освобождение временных данных
    for (size_t i = 0; i < file_count; i++) {
        free(hash_array[i].hash);
    }
    free(hash_array);
    free(file_list);

    file_list = filtered_list;
    file_count = filtered_count;
    file_list_capacity = filtered_capacity;
}
