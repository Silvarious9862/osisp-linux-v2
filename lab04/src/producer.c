#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include "common.h"
#include "ipc.h"

// Глобальная переменная для обработки завершения процесса
volatile sig_atomic_t terminate_flag = 0;

// Обработчик сигнала SIGTERM
void termination_handler(int sig) {
    terminate_flag = 1;
}

int main() {
    // Регистрация обработчика SIGTERM
    signal(SIGTERM, termination_handler);

    // Подключение к разделяемой памяти
    int shm_id = shmget(SHM_KEY, sizeof(MessageQueue), 0666);
    if (shm_id == -1) {
        perror("Ошибка подключения к общей памяти");
        exit(1);
    }

    MessageQueue* queue = (MessageQueue*) shmat(shm_id, NULL, 0);
    if (queue == (void*)-1) {
        perror("Ошибка подключения к очереди");
        exit(1);
    }

    // Подключение к семафорам
    int sem_id = semget(SEM_KEY, 3, 0666);
    if (sem_id == -1) {
        perror("Ошибка подключения к семафорам");
        exit(1);
    }

    srand(time(NULL) ^ getpid());
    printf("Производитель %d запущен.\n", getpid());

    while (1) {
        // Проверка флага завершения
        if (terminate_flag) {
            printf("Производитель %d завершает работу.\n", getpid());
            break;
        }

        // Генерация нового сообщения
        Message message;
        message.type = 'A' + (rand() % 26);

        // Определение размера сообщения с обработкой особого случая (256 байт)
        int r = rand() % 257;
        while (r == 0) {
            r = rand() % 257;
        }
        
        int actual_len = (r == 256) ? 256 : r;
        message.size = (r == 256) ? 0 : r;

        // Вычисление выровненного размера данных
        int padded_len = ((actual_len + 3) / 4) * 4;

        // Заполнение данных случайными буквами с нулевыми дополнениями
        for (int i = 0; i < padded_len; i++) {
            message.data[i] = (i < actual_len) ? ('A' + (rand() % 26)) : 0;
        }

        // Вычисление контрольной суммы
        message.hash = calculate_hash(&message);

        // Ожидание доступного места и захват мьютекса
        semaphore_op(sem_id, 0, -1);
        semaphore_op(sem_id, 2, -1);

        // Добавление сообщения в очередь
        queue->buffer[queue->tail] = message;
        queue->tail = (queue->tail + 1) % QUEUE_SIZE;
        queue->free_slots--;
        queue->added_count++;

        printf("Производитель %d: добавлено сообщение (тип '%c', ", getpid(), message.type);
        printf("размер %d, hash %u). Всего добавлено: %d\n",
               (message.size == 0) ? 256 : message.size, message.hash, queue->added_count);

        // Освобождение мьютекса и индикация заполненного слота
        semaphore_op(sem_id, 2, 1);
        semaphore_op(sem_id, 1, 1);

        sleep(3);
    }

    return 0;
}
