#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <openssl/md5.h>
#include "verbose.h"

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

// Вычисление MD5-хеша файла
static char *compute_md5(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("Ошибка открытия файла");
        return NULL;
    }

    MD5_CTX ctx;
    if (MD5_Init(&ctx) != 1) {
        fclose(fp);
        fprintf(stderr, "Ошибка инициализации MD5\n");
        return NULL;
    }

    unsigned char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        if (MD5_Update(&ctx, buffer, bytes) != 1) {
            fclose(fp);
            fprintf(stderr, "Ошибка вычисления MD5\n");
            return NULL;
        }
    }
    fclose(fp);

    unsigned char digest[MD5_DIGEST_LENGTH];
    if (MD5_Final(digest, &ctx) != 1) {
        fprintf(stderr, "Ошибка завершения вычисления MD5\n");
        return NULL;
    }

    char *md5_str = malloc(MD5_DIGEST_LENGTH * 2 + 1);
    if (!md5_str) {
        perror("Ошибка выделения памяти для MD5");
        return NULL;
    }
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(&md5_str[i * 2], "%02x", digest[i]);
    }
    md5_str[MD5_DIGEST_LENGTH * 2] = '\0';
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

    // Сортировка файлов по MD5
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

        // Если MD5 совпадает у нескольких файлов, добавляем их в список
        if ((j - i) > 1) {
            for (size_t k = i; k < j; k++) {
                if (filtered_count >= filtered_capacity) {
                    filtered_capacity = filtered_capacity ? filtered_capacity * 2 : 10;
                    filtered_list = realloc(filtered_list, filtered_capacity * sizeof(file_entry));
                    if (!filtered_list) {
                        perror("Ошибка выделения памяти для filtered_list");
                        exit(EXIT_FAILURE);
                    }
                }
                filtered_list[filtered_count++] = hash_array[k].entry;
            }
        } else {
            verbose_log_path(file_list[i].full_path);
            unique_flag = 1;
        }
        i = j;
    }
    if (!unique_flag) verbose_log("Не найдены");

    // Очистка временных данных
    for (size_t i = 0; i < file_count; i++) {
        free(hash_array[i].hash);
    }
    free(hash_array);
    free(file_list);

    file_list = filtered_list;
    file_count = filtered_count;
    file_list_capacity = filtered_capacity;
}
