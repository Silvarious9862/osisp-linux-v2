#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "args.h"

Options parse_arguments(int argc, char *argv[]) {
    Options opts = {0, 0, 0, 0, 0, 0, 0, 0, 0, NULL};
    int index = 1;

    // Разбор аргументов командной строки
    while (index < argc && argv[index][0] == '-' && argv[index][1] != '\0') {
        if (strcmp(argv[index], "--") == 0) {  // Завершение обработки опций
            index++;
            break;
        }

        // Чтение символов флага
        for (int i = 1; argv[index][i] != '\0'; i++) {
            switch (argv[index][i]) {
                case 'r': opts.recursive = 1;   break;
                case 't': opts.show_time = 1;   break;
                case 'm': opts.summary = 1;     break;
                case 'S': opts.show_size = 1;   break;
                case 'v': opts.verbose = 1;     break;
                case 'd': opts.deletion = 1;    break;
                case 'n': opts.no_interact = 1; break;
                case 'G': {  // Установка минимального размера
                    char *value = &argv[index][i+1];
                    if (*value == '\0') {
                        if (++index >= argc) {
                            fprintf(stderr, "Не указан аргумент для -G\n");
                            exit(EXIT_FAILURE);
                        }
                        value = argv[index];
                    }
                    opts.min_size = strtoull(value, NULL, 10);
                    i = strlen(argv[index]) - 1;
                    break;
                }
                case 'L': {  // Установка максимального размера
                    char *value = &argv[index][i+1];
                    if (*value == '\0') {
                        if (++index >= argc) {
                            fprintf(stderr, "Не указан аргумент для -L\n");
                            exit(EXIT_FAILURE);
                        }
                        value = argv[index];
                    }
                    opts.max_size = strtoull(value, NULL, 10);
                    i = strlen(argv[index]) - 1;
                    break;
                }
                default:
                    fprintf(stderr, "Неизвестный флаг: -%c\n", argv[index][i]);
                    exit(EXIT_FAILURE);
            }
        }
        index++;
    }

    if (index >= argc) {
        fprintf(stderr, "Ошибка: не указан начальный путь\n");
        exit(EXIT_FAILURE);
    }
    opts.start_path = argv[index];

    // Проверка корректности пути
    struct stat statbuf;
    if (stat(opts.start_path, &statbuf) != 0 || !S_ISDIR(statbuf.st_mode)) {
        fprintf(stderr, "Указанный путь некорректен или не является директорией: %s\n", opts.start_path);
        exit(EXIT_FAILURE);
    }

    return opts;
}
