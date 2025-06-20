#ifndef ARGS_H
#define ARGS_H

// Структура для хранения параметров командной строки
typedef struct {
    int recursive;   // Включить рекурсивное сканирование
    int show_time;   // Отображать время модификации файлов
    int summary;     // Вывести сводную информацию
    int show_size;   // Показывать размер файлов
    int verbose;     // Режим подробного вывода
    int deletion;    // Удалять найденные дубликаты
    int no_interact; // Отключить интерактивный режим
    unsigned long long max_size; // Максимальный размер файла
    unsigned long long min_size; // Минимальный размер файла
    const char *start_path; // Начальный путь для сканирования
} Options;

// Разбор аргументов командной строки
Options parse_arguments(int argc, char *argv[]);

#endif  // ARGS_H
