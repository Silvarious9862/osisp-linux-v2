#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <magic.h>
#include "verbose.h"

// Структура для хранения информации о файле
typedef struct {
    char full_path[PATH_MAX];
    off_t file_size;
} file_entry;

// Глобальные переменные
extern file_entry *file_list;
extern size_t file_count;
extern size_t file_list_capacity;

// Прототипы функций модуля
void filter_mime_list(void);
void print_filtered_file_list(void);

// Фильтрация списка файлов по MIME-типу
void filter_mime_list(void) {
    if (file_count == 0)
        return;

    // Инициализация libmagic
    magic_t cookie = magic_open(MAGIC_MIME);
    if (!cookie) {
        fprintf(stderr, "Ошибка инициализации libmagic\n");
        exit(EXIT_FAILURE);
    }
    if (magic_load(cookie, NULL) != 0) {
        fprintf(stderr, "Ошибка загрузки базы libmagic: %s\n", magic_error(cookie));
        magic_close(cookie);
        exit(EXIT_FAILURE);
    }

    file_entry *filtered_list = NULL;
    size_t filtered_count = 0, filtered_capacity = 0;
    int unique_flag = 0;
    verbose_log("Уникальные файлы:");

    size_t i = 0;
    while (i < file_count) {
        // Определение группы файлов с одинаковым размером
        size_t j = i + 1;
        while (j < file_count && file_list[j].file_size == file_list[i].file_size)
            j++;
        size_t group_size = j - i;

        // Выделение памяти под MIME-типы группы
        char **mime_array = malloc(group_size * sizeof(char *));
        if (!mime_array) {
            perror("Ошибка выделения памяти");
            magic_close(cookie);
            exit(EXIT_FAILURE);
        }

        // Получение MIME-типов для файлов
        for (size_t k = 0; k < group_size; k++) {
            const char *mime = magic_file(cookie, file_list[i + k].full_path);
            mime_array[k] = mime ? strdup(mime) : NULL;
            if (!mime_array[k] && mime) {
                perror("Ошибка выделения памяти для MIME");
                for (size_t m = 0; m < k; m++)
                    free(mime_array[m]);
                free(mime_array);
                magic_close(cookie);
                exit(EXIT_FAILURE);
            }
        }

        // Подсчёт повторений MIME-типа внутри группы
        for (size_t k = 0; k < group_size; k++) {
            if (!mime_array[k])
                continue;
            int count = 0;
            for (size_t l = 0; l < group_size; l++) {
                if (mime_array[l] && strcmp(mime_array[k], mime_array[l]) == 0)
                    count++;
            }
            // Добавление файлов с повторяющимся MIME в список
            if (count > 1) {
                if (filtered_count >= filtered_capacity) {
                    filtered_capacity = filtered_capacity ? filtered_capacity * 2 : 10;
                    file_entry *temp = realloc(filtered_list, filtered_capacity * sizeof(file_entry));
                    if (!temp) {
                        perror("Ошибка выделения памяти");
                        for (size_t m = 0; m < group_size; m++)
                            free(mime_array[m]);
                        free(mime_array);
                        magic_close(cookie);
                        exit(EXIT_FAILURE);
                    }
                    filtered_list = temp;
                }
                filtered_list[filtered_count++] = file_list[i + k];
            } else {
                // Файл считается уникальным – выводим его путь
                verbose_log_path(file_list[i].full_path);
                unique_flag = 1;
            }
        }

        // Освобождение памяти MIME-массивов
        for (size_t k = 0; k < group_size; k++)
            free(mime_array[k]);
        free(mime_array);

        i = j;
    }

    if (!unique_flag)
        verbose_log("Не найдены");

    free(file_list);
    file_list = filtered_list;
    file_count = filtered_count;
    file_list_capacity = filtered_capacity;  // Обновление вместимости списка

    magic_close(cookie);
}
