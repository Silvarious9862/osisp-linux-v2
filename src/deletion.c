#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <limits.h>

// Структура файла с его полным путем и размером
typedef struct {
    char full_path[PATH_MAX];
    off_t file_size;
} file_entry;

// Глобальные переменные, определённые в selection.c
extern file_entry *file_list;
extern size_t file_count;
extern size_t file_list_capacity;
extern int time_flag;

// Интерактивное удаление дубликатов
void interactive_delete_duplicates(void) {
    if (file_count == 0) {
        printf("Нет дубликатов для удаления.\n");
        return;
    }

    // Подсчет количества наборов дубликатов
    size_t total_sets = 0, idx = 0;
    while (idx < file_count) {
        size_t groupCount = 0;
        off_t current_size = file_list[idx].file_size;
        while (idx < file_count && file_list[idx].file_size == current_size) {
            groupCount++;
            idx++;
        }
        if (groupCount >= 2)
            total_sets++;
    }

    // Повторный проход по списку
    idx = 0;
    size_t set_number = 0;
    
    while (idx < file_count) {
        size_t group_start = idx;
        off_t current_size = file_list[idx].file_size;
        size_t groupCount = 0;
        while (idx < file_count && file_list[idx].file_size == current_size) {
            groupCount++;
            idx++;
        }
        if (groupCount < 2)
            continue;

        set_number++; // Нумерация групп

        // Вывод списка файлов в группе
        for (size_t j = 0; j < groupCount; j++) {
            struct stat sb;
            if (time_flag && stat(file_list[group_start + j].full_path, &sb) == 0) {
                char timebuf[32];
                struct tm *tm_info = localtime(&sb.st_mtime);
                strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M", tm_info);
                printf("[%zu] [%s] %s\n", j + 1, timebuf, file_list[group_start + j].full_path);
            } else {
                printf("[%zu] %s\n", j + 1, file_list[group_start + j].full_path);
            }
        }

        // Запрос выбора файла для сохранения
        char input[32];
        int keep_index = -1;
        while (1) {
            printf("\nНабор %zu из %zu, сохранить файл [1 - %zu, all, quit]: ", set_number, total_sets, groupCount);
            if (!fgets(input, sizeof(input), stdin))
                continue;
            input[strcspn(input, "\n")] = '\0';
            if (strcmp(input, "quit") == 0)
                return;
            if (strcmp(input, "all") == 0)
                break;
            keep_index = atoi(input) - 1;
            if (keep_index >= 0 && (size_t)keep_index < groupCount)
                break;
            printf("Неверный выбор, попробуйте снова.\n");
        }

        // Удаление всех файлов, кроме выбранного
        if (strcmp(input, "all") != 0) {
            for (size_t j = 0; j < groupCount; j++) {
                if (j == (size_t)keep_index)
                    continue;
                if (remove(file_list[group_start + j].full_path) == 0)
                    printf("Удален: %s\n", file_list[group_start + j].full_path);
                else
                    perror("Ошибка удаления");
            }
        }
        printf("\n");
    }
}

// Неинтерактивное удаление дубликатов
void non_interactive_delete_duplicates(void) {
    if (file_count == 0) {
        printf("Нет дубликатов для удаления.\n");
        return;
    }

    size_t idx = 0;
    while (idx < file_count) {
        size_t group_start = idx;
        off_t current_size = file_list[idx].file_size;
        size_t groupCount = 0;

        // Группировка файлов по одинаковому размеру
        while (idx < file_count && file_list[idx].file_size == current_size) {
            groupCount++;
            idx++;
        }

        // Пропуск групп с одним файлом
        if (groupCount < 2)
            continue;

        printf("   [+] %s\n", file_list[group_start].full_path);

        // Удаление всех файлов, кроме первого в группе
        for (size_t j = group_start + 1; j < group_start + groupCount; j++) {
            if (remove(file_list[j].full_path) == 0)
                printf("   [-] %s\n", file_list[j].full_path);
            else
                perror("Ошибка удаления");
        }
        printf("\n");
    }
}
