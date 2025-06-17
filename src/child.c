#include "child.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

// Глобальные переменные для обработки сигналов
volatile sig_atomic_t sigusr1_triggered = 0;
volatile sig_atomic_t print_permission = 0;
int combinations[4] = {0, 0, 0, 0};

// Обработчик SIGUSR1 – устанавливает флаг для обновления статистики
void sigusr1_handler(int sig) {
    if (sig == SIGUSR1) {
        sigusr1_triggered = 1;
    }
}

// Обработчик SIGUSR2 – разрешает вывод статистики
void sigusr2_handler(int sig) {
    if (sig == SIGUSR2) {
        print_permission = 1;
    }
}

// Обработка данных: вычисление индекса для статистики
void process_data(Data *data) {
    int index = (data->x << 1) | data->y;
    if (index >= 0 && index < 4) {
        combinations[index]++;
    }
}

// Вывод статистики дочернего процесса
void print_statistics(pid_t ppid, pid_t pid) {
    printf("Child: PPID=%d, PID=%d, {0,0}=%d, {1,1}=%d, {0,1}=%d, {1,0}=%d\n",
           ppid, pid, combinations[0], combinations[1], combinations[2], combinations[3]);
    fflush(stdout);
}

int main() {
    // Настройка обработчиков сигналов SIGUSR1 и SIGUSR2
    struct sigaction sa1, sa2;

    sa1.sa_handler = sigusr1_handler;
    sa1.sa_flags = 0;
    sigemptyset(&sa1.sa_mask);
    if (sigaction(SIGUSR1, &sa1, NULL) == -1) {
        perror("Ошибка установки обработчика SIGUSR1");
        exit(EXIT_FAILURE);
    }

    sa2.sa_handler = sigusr2_handler;
    sa2.sa_flags = 0;
    sigemptyset(&sa2.sa_mask);
    if (sigaction(SIGUSR2, &sa2, NULL) == -1) {
        perror("Ошибка установки обработчика SIGUSR2");
        exit(EXIT_FAILURE);
    }

    // Инициализация генератора случайных чисел
    srand(time(NULL));

    Data data = {0, 0};
    int iterations = 0;
    const int THRESHOLD = 101; // Число итераций перед выводом статистики

    while (1) {
        // Задержка перед генерацией случайных данных (100 мс)
        struct timespec ts = {0, 100000000};
        nanosleep(&ts, NULL);

        // Генерация случайных значений (0 или 1)
        data.x = rand() % 2;
        data.y = rand() % 2;

        // Имитируем обновление статистики с помощью SIGUSR1
        raise(SIGUSR1);

        if (sigusr1_triggered) {
            sigusr1_triggered = 0;
            process_data(&data);
            iterations++;
        }

        // Ожидание SIGUSR2 для вывода статистики после определённого количества итераций
        if (iterations % THRESHOLD == 0) {
            while (!print_permission) {
                pause();
            }
            print_statistics(getppid(), getpid());
            print_permission = 0;  // Сброс флага для следующего цикла
        }
    }

    return 0;
}
