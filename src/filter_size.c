#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <time.h>
#include "result.h"
#include "verbose.h"

// Глобальные переменные
extern file_entry *file_list;
extern size_t file_list_capacity;
extern size_t file_count;  // Предполагается, что file_count объявлена в result.h

// Прототипы функций
int compare_file_entry(const void *a, const void *b);
void filter_file_list(void);
void print_filtered_file_list(void);

// Сравнение файлов для сортировки по размеру
int compare_file_entry(const void *a, const void *b) {
    const file_entry *fa = (const file_entry *)a;
    const file_entry *fb = (const file_entry *)b;
    if (fa->file_size < fb->file_size)
        return -1;
    else if (fa->file_size > fb->file_size)
        return 1;
    else 
        return 0;
}

// Фильтрация списка файлов, оставляя только дубликаты по размеру
void filter_file_list(void) {
    if (file_count == 0)
        return;

    // Сортировка списка по размеру файлов
    qsort(file_list, file_count, sizeof(file_entry), compare_file_entry);

    file_entry *filtered_list = NULL;
    size_t filtered_count = 0, filtered_capacity = 0;
    int unique_flag = 0;
    verbose_log("Уникальные файлы:");

    for (size_t i = 0; i < file_count;) {
        size_t j = i + 1;
        
        // Группировка файлов с одинаковым размером
        while (j < file_count && file_list[j].file_size == file_list[i].file_size)
            j++;

        // Если в группе больше одного файла – добавляем их в новый список
        if ((j - i) > 1) {
            size_t group_size = j - i;
            if (filtered_count + group_size > filtered_capacity) {
                filtered_capacity = filtered_capacity ? filtered_capacity * 2 : 10;
                file_entry *temp = realloc(filtered_list, filtered_capacity * sizeof(file_entry));
                if (!temp) {
                    perror("Ошибка выделения памяти");
                    free(filtered_list);
                    exit(EXIT_FAILURE);
                }
                filtered_list = temp;
            }
            for (size_t k = i; k < j; k++)
                filtered_list[filtered_count++] = file_list[k];
        } else {
            // Если файл уникален – выводим его путь
            verbose_log_path(file_list[i].full_path);
            unique_flag = 1;
        }
        i = j;
    }
    
    if (!unique_flag) 
        verbose_log("Не найдены");

    // Освобождаем оригинальный список и заменяем его на отфильтрованный
    free(file_list);
    file_list = filtered_list;
    file_count = filtered_count;
    file_list_capacity = filtered_capacity; // обновляем значение вместимости
}
