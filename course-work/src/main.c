#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "args.h"
#include "result.h"
#include "verbose.h"

// Прототипы функций
void scan_directory(const char *path);
void filter_size_list(void);
void filter_file_list(void);
void filter_mime_list(void);
void filter_hash_list(void);
void filter_cmp_list(void);
void print_filtered_file_list(void);
void interactive_delete_duplicates(void);
void non_interactive_delete_duplicates(void);
void cleanup_selection(void);

// Определение структуры фильтрации
typedef void (*filter_func_t)(void);
typedef struct {
    const char *message;
    filter_func_t func;
} Filter;

Filter filters[] = {
    {"Фильтрация по объему памяти...", filter_file_list},
    {"Фильтрация по типу файла...", filter_mime_list},
    {"Фильтрация по хешу...", filter_hash_list},
    {"Фильтрация побайтовым сравнением...", filter_cmp_list}
};

size_t num_filters = sizeof(filters) / sizeof(filters[0]);

// Глобальные флаги и параметры
int recursive_flag = 0;
int time_flag = 0;
int summary_flag = 0;
int size_flag = 0;
int verbose_flag = 0;
int deletion_flag = 0;
int no_interact_flag = 0;
unsigned long long min_size = 0;
unsigned long long max_size = 0;

int main(int argc, char *argv[]) {
    atexit(cleanup_selection);

    Options opts = parse_arguments(argc, argv);

    // Инициализация параметров
    recursive_flag = opts.recursive;
    time_flag = opts.show_time;
    summary_flag = opts.summary;
    size_flag = opts.show_size;
    verbose_flag = opts.verbose;
    deletion_flag = opts.deletion;
    no_interact_flag = opts.no_interact;
    min_size = opts.min_size;
    max_size = opts.max_size;

    if (deletion_flag && summary_flag) {
        printf("Флаги -m и -d несовместимы\n");
        return EXIT_FAILURE;
    }

    scan_directory(opts.start_path);
    filter_size_list();

    // Поэтапная фильтрация файлов
    for (size_t i = 0; i < num_filters; i++) {
        if (file_count == 0) {
            verbose_log("Список файлов пуст.");
            break;
        }
        verbose_log(filters[i].message);
        filters[i].func();
    }

    // Вывод или удаление файлов в зависимости от флагов
    
    if (deletion_flag && no_interact_flag) 
        non_interactive_delete_duplicates();
    else if (deletion_flag) 
        interactive_delete_duplicates();
    else if (summary_flag) 
        print_summary();
    else if (time_flag && size_flag) 
        print_size_time_listing();
    else if (size_flag) 
        print_size_listing();
    else 
        print_filtered_file_list();

    return EXIT_SUCCESS;
}
