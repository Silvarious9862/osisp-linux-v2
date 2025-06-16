#ifndef OPTIONS_H
#define OPTIONS_H

// Структура для хранения параметров командной строки
typedef struct {
    int type_links; // Включение обработки символических ссылок (-l)
    int type_dirs;  // Включение обработки каталогов (-d)
    int type_files; // Включение обработки обычных файлов (-f)
    int sort;       // Активация сортировки по LC_COLLATE (-s)
} Options;

// Функция разбора аргументов командной строки
void parse_options(int argc, char *argv[], char **start_dir, Options *opts);

#endif // OPTIONS_H
