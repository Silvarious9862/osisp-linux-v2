#include "dirwalk.h"
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <locale.h>
#include <signal.h>

#define INITIAL_CAPACITY 128

// Структура списка найденных путей
typedef struct {
    char **items;
    size_t size;
    size_t capacity;
} FileList;

// Глобальный список файлов для очистки при SIGINT
static FileList *g_file_list = NULL;

// Обработчик SIGINT: очищает память и завершает программу
static void sigint_handler(int signum) {
    (void)signum; // Избегаем предупреждений
    if (g_file_list) {
        free_file_list(g_file_list);
        g_file_list = NULL;
    }
    fprintf(stderr, "\nПолучен SIGINT (Ctrl-C). Освобождаю ресурсы и завершаю работу...\n");
    exit(EXIT_FAILURE);
}

// Регистрация обработчика SIGINT
static void register_sigint_handler(void) {
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // Можно добавить SA_RESTART
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

// Создание списка файлов
static FileList* create_file_list(void) {
    FileList *list = malloc(sizeof(FileList));
    if (!list) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    list->capacity = INITIAL_CAPACITY;
    list->size = 0;
    list->items = malloc(list->capacity * sizeof(char *));
    if (!list->items) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    return list;
}

// Добавление пути в список
static void add_to_list(FileList *list, const char *path) {
    if (list->size >= list->capacity) {
        list->capacity *= 2;
        list->items = realloc(list->items, list->capacity * sizeof(*list->items));
        if (!list->items) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
    }
    list->items[list->size] = strdup(path);
    if (!list->items[list->size]) {
        perror("strdup");
        exit(EXIT_FAILURE);
    }
    list->size++;
}

// Сравнение строк для сортировки
static int cmp_str(const void *a, const void *b) {
    return strcoll(*(const char **)a, *(const char **)b);
}

// Освобождение списка файлов
static void free_file_list(FileList *list) {
    for (size_t i = 0; i < list->size; i++) {
        free(list->items[i]);
    }
    free(list->items);
    free(list);
}

// Вывод списка файлов
static void print_file_list(const FileList *list) {
    for (size_t i = 0; i < list->size; i++) {
        printf("%s\n", list->items[i]);
    }
}

// Проверка соответствия файла заданным опциям
static int match_type(const char *path, const Options *opts) {
    struct stat sb;
    if (lstat(path, &sb) == -1) {
        perror(path);
        return 0;
    }
    if (!opts->type_files && !opts->type_dirs && !opts->type_links)
        return 1;

    if (opts->type_files && S_ISREG(sb.st_mode))
        return 1;
    if (opts->type_dirs && S_ISDIR(sb.st_mode))
        return 1;
    if (opts->type_links && S_ISLNK(sb.st_mode))
        return 1;

    return 0;
}

// Рекурсивный обход каталога
static void dirwalk_internal(const char *dir, const Options *opts, FileList *list) {
    DIR *dp = opendir(dir);
    if (!dp) {
        perror(dir);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        size_t dir_len = strlen(dir);
        int need_slash = (dir[dir_len - 1] == '/') ? 0 : 1;
        size_t path_len = dir_len + need_slash + strlen(entry->d_name) + 1;

        char *path = malloc(path_len);
        if (!path) {
            perror("malloc");
            closedir(dp);
            exit(EXIT_FAILURE);
        }
        snprintf(path, path_len, "%s%s%s", dir, need_slash ? "/" : "", entry->d_name);

        if (match_type(path, opts))
            add_to_list(list, path);

        struct stat sb;
        if (lstat(path, &sb) == 0 && S_ISDIR(sb.st_mode))
            dirwalk_internal(path, opts, list);

        free(path);
    }
    closedir(dp);
}

// Запуск обхода каталога
void dirwalk(const char *start_dir, const Options *opts) {
    register_sigint_handler();

    if (opts->sort)
        setlocale(LC_COLLATE, "");

    FileList *list = create_file_list();
    g_file_list = list;

    dirwalk_internal(start_dir, opts, list);

    if (opts->sort)
        qsort(list->items, list->size, sizeof(char *), cmp_str);

    print_file_list(list);
    free_file_list(list);
    g_file_list = NULL;
}
