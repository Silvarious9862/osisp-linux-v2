#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "options.h"

// Функция отображения справки и завершения программы
static void usage(const char *progname) {
    fprintf(stderr, "Usage: %s [dir] [options]\n", progname);
    exit(EXIT_FAILURE);
}

// Разбор аргументов командной строки
void parse_options(int argc, char *argv[], char **start_dir, Options *opts) {
    // Инициализация флагов
    opts->type_links = 0;
    opts->type_dirs  = 0;
    opts->type_files = 0;
    opts->sort       = 0;

    int opt;
    while ((opt = getopt(argc, argv, "ldfs")) != -1) {
        switch (opt) {
            case 'l':
                opts->type_links = 1; // Включение обработки ссылок
                break;
            case 'd':
                opts->type_dirs = 1; // Включение обработки директорий
                break;
            case 'f':
                opts->type_files = 1; // Включение обработки файлов
                break;
            case 's':
                opts->sort = 1; // Включение сортировки
                break;
            default:
                usage(argv[0]); // Вызов справки при некорректных аргументах
        }
    }

    // Определение начального каталога
    *start_dir = (optind < argc) ? argv[optind] : "./";
}
