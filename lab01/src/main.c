#include <stdio.h>
#include <stdlib.h>
#include "options.h"
#include "dirwalk.h"

// Главная функция программы
int main(int argc, char *argv[]) {
    char *start_dir;
    Options opts;

    // Разбор аргументов командной строки
    parse_options(argc, argv, &start_dir, &opts);

    // Запуск рекурсивного обхода файлов
    dirwalk(start_dir, &opts);

    return 0;
}
