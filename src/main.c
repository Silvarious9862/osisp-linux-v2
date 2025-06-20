#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include "common.h"
#include "thread_queue.h"
#include "producer_consumer.h"

#define MAX_THREADS 100

// Глобальные переменные управления очередью и потоками
volatile int terminate_flag = 0;
ThreadMessageQueue queue;
pthread_mutex_t resize_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int pause_processing = 0;

// Вывод текущего состояния очереди
void print_status() {
    pthread_mutex_lock(&queue.mutex);
    printf("\n--- Состояние очереди ---\n");
    printf("Ёмкость очереди: %d\n", queue.capacity);
    printf("Элементов в очереди: %d\n", queue.count);
    printf("Свободных слотов: %d\n", queue.capacity - queue.count);
    printf("Добавлено сообщений: %d\n", queue.added_count);
    printf("Извлечено сообщений: %d\n", queue.removed_count);
    pthread_mutex_unlock(&queue.mutex);
}

// Обработчик SIGINT (Ctrl-C) для корректного завершения
static void sigint_handler(int sig) {
    (void)sig;
    printf("\nПолучен SIGINT. Запускается завершение работы...\n");
    terminate_flag = 1;
}

// Поток для мониторинга возможных дедлоков
void* deadlock_monitor(void* arg) {
    while (!terminate_flag) {
        if (queue.count == 0 && queue.added_count > 0)
            printf("--- Нет элементов в очереди, проверьте работу производителей ---\n");
        sleep(5);
    }
    return NULL;
}

int main() {
    // Регистрация обработчика SIGINT
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Ошибка sigaction");
        exit(1);
    }
    
    // Инициализация очереди
    srand(time(NULL));
    if (init_thread_queue(&queue, 10) != 0) {
        perror("Ошибка инициализации очереди");
        return 1;
    }
    
    pthread_t producers[MAX_THREADS], consumers[MAX_THREADS];
    int prod_count = 0, cons_count = 0;
    char command[10];

    // Обновлённая справка: добавлены команды для удаления потоков
    printf("Команды:\n"
           "  + : добавить поток-производитель\n"
           "  - : добавить поток-потребитель\n"
           "  P : удалить последний поток-производитель\n"
           "  C : удалить последний поток-потребитель\n"
           "  > : увеличить размер очереди\n"
           "  < : уменьшить размер очереди\n"
           "  p : вывести состояние очереди\n"
           "  q : завершение работы\n");
    
    // Запуск потока мониторинга
    pthread_t monitor_thread;
    pthread_create(&monitor_thread, NULL, deadlock_monitor, NULL);
    
    while (fgets(command, sizeof(command), stdin) != NULL) {
        if (command[0] == 'q') {
            terminate_flag = 1;
            break;
        } else if (command[0] == '+') {
            int *id = malloc(sizeof(int));
            if (!id) continue;
            *id = prod_count + 1;
            if (pthread_create(&producers[prod_count], NULL, producer_thread, id) != 0) {
                perror("Ошибка создания потока-производителя");
                free(id);
            } else {
                prod_count++;
            }
        } else if (command[0] == '-') {
            int *id = malloc(sizeof(int));
            if (!id) continue;
            *id = cons_count + 1;
            if (pthread_create(&consumers[cons_count], NULL, consumer_thread, id) != 0) {
                perror("Ошибка создания потока-потребителя");
                free(id);
            } else {
                cons_count++;
            }
        }
        // Новая команда: удаление последнего добавленного потока-производителя (LIFO)
        else if (command[0] == 'P') {
            if (prod_count > 0) {
                if (pthread_cancel(producers[prod_count - 1]) == 0) {
                    pthread_join(producers[prod_count - 1], NULL);
                    prod_count--;
                    printf("Удален поток-производитель [%d].\n", prod_count+1);
                } else {
                    printf("Ошибка при удалении потока-производителя.\n");
                }
            } else {
                printf("Нет активных потоков-производителей для удаления.\n");
            }
        }
        // Новая команда: удаление последнего добавленного потока-потребителя (LIFO)
        else if (command[0] == 'C') {
            if (cons_count > 0) {
                if (pthread_cancel(consumers[cons_count - 1]) == 0) {
                    pthread_join(consumers[cons_count - 1], NULL);
                    cons_count--;
                    printf("Удален поток-потребитель [%d].\n", cons_count+1);
                } else {
                    printf("Ошибка при удалении потока-потребителя.\n");
                }
            } else {
                printf("Нет активных потоков-потребителей для удаления.\n");
            }
        } else if (command[0] == 'p') {
            print_status();
        } else if (command[0] == '>') {
            int new_capacity = queue.capacity + 1;
            if (resize_thread_queue(&queue, new_capacity) == 0)
                printf("Очередь увеличена до %d слотов\n", new_capacity);
            else
                printf("Ошибка при увеличении очереди\n");
        } else if (command[0] == '<') {
            /*if (queue.count != 0) {
                printf("Нельзя уменьшить очередь, пока она не пуста (занято %d элементов)\n", queue.count);
                continue;
            }*/
            if (queue.capacity <= 2) {
                printf("Минимальная ёмкость очереди равна 2. Уменьшение невозможно.\n");
                continue;
            }
            int new_capacity = queue.capacity - 1;
            pause_processing = 1;
            sleep(1); // небольшая задержка перед уменьшением
            if (resize_thread_queue(&queue, new_capacity) == 0)
                printf("Очередь уменьшена до %d слотов\n", new_capacity);
            else
                printf("Ошибка при уменьшении очереди\n");
            pause_processing = 0;
        } else {
            printf("Неизвестная команда: %s", command);
        }
    }
    
    // Ожидание завершения всех потоков
    for (int i = 0; i < prod_count; i++) {
        pthread_join(producers[i], NULL);
    }
    for (int i = 0; i < cons_count; i++) {
        pthread_join(consumers[i], NULL);
    }
    
    pthread_join(monitor_thread, NULL);
    
    destroy_thread_queue(&queue);
    printf("Программа завершена.\n");
    return 0;
}
