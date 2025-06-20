#ifndef PARENT_H
#define PARENT_H

#include <sys/types.h>

#define MAX_CHILDREN 100 // Максимальное количество дочерних процессов

// Массив PID дочерних процессов и их количество
extern pid_t children[MAX_CHILDREN];
extern int child_count;

// Функции управления дочерними процессами
void add_child(void);           // Создать новый дочерний процесс
void remove_last_child(void);   // Завершить последний дочерний процесс
void list_processes(void);      // Вывести список всех процессов
void kill_all_children(void);   // Завершить все дочерние процессы
void handle_input(void);        // Обработка пользовательского ввода

#endif /* PARENT_H */
