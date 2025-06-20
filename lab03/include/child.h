#ifndef CHILD_H
#define CHILD_H

#include <sys/types.h>
#include <signal.h>

// Структура для хранения данных (значения 0 или 1)
typedef struct {
    int x;
    int y;
} Data;

// Глобальные переменные для обработки сигналов и статистики
extern volatile sig_atomic_t sigusr1_triggered; // Флаг для обработки SIGUSR1
extern volatile sig_atomic_t print_permission;  // Флаг для разрешения вывода статистики (SIGUSR2)
extern int combinations[4]; // Хранение количества комбинаций {0,0}, {1,1}, {0,1}, {1,0}

// Прототипы функций обработки сигналов и статистики
void sigusr1_handler(int sig); // Обработчик SIGUSR1
void sigusr2_handler(int sig); // Обработчик SIGUSR2
void process_data(Data *data); // Обновление статистики по сгенерированным значениям
void print_statistics(pid_t ppid, pid_t pid); // Вывод статистики дочернего процесса

#endif /* CHILD_H */
