#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include "common.h"
#include "queue.h"
#include "producer_consumer.h"

#define MAX_THREADS 100

// Глобальные переменные
volatile int terminate_flag = 0;
MessageQueue queue;

// Обработчик SIGINT (Ctrl-C) для завершения работы
static void sigint_handler(int sig) {
    (void)sig; 
    printf("\nПолучен SIGINT. Запускается завершение работы...\n");
    terminate_flag = 1;
    pthread_mutex_lock(&queue.mutex);
    pthread_cond_broadcast(&queue.not_empty);
    pthread_cond_broadcast(&queue.not_full);
    pthread_mutex_unlock(&queue.mutex);
}

int main() {
    // Настройка обработчика SIGINT
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
    if (init_queue(&queue, INITIAL_CAPACITY) != 0) {
        perror("Ошибка инициализации очереди");
        return 1;
    }

    pthread_t producers[MAX_THREADS], consumers[MAX_THREADS];
    int prod_count = 0, cons_count = 0;
    char command[10];

    printf("Команды:\n"
           "  + : добавить поток-производитель\n"
           "  - : добавить поток-потребитель\n"
           "  > : увеличить размер очереди\n"
           "  < : уменьшить размер очереди (только если очередь пуста)\n"
           "  d : удалить производителя\n"
           "  f : удалить потребителя\n"
           "  p : вывести состояние очереди\n"
           "  q : завершение работы\n");

    while (fgets(command, sizeof(command), stdin) != NULL) {
        if (command[0] == 'q') {
            terminate_flag = 1;
            pthread_mutex_lock(&queue.mutex);
            pthread_cond_broadcast(&queue.not_empty);
            pthread_cond_broadcast(&queue.not_full);
            pthread_mutex_unlock(&queue.mutex);
            break;
        } else if (command[0] == '+') {
            int *id = malloc(sizeof(int));
            if (!id) continue;
            *id = ++prod_count;  // присваиваем новый номер производителя
            if (pthread_create(&producers[prod_count - 1], NULL, producer_thread, id) != 0) {
                perror("Ошибка создания производителя");
                free(id);
            }
        } else if (command[0] == '-') {
            int *id = malloc(sizeof(int));
            if (!id) continue;
            *id = ++cons_count;  // присваиваем новый номер потребителя
            if (pthread_create(&consumers[cons_count - 1], NULL, consumer_thread, id) != 0) {
                perror("Ошибка создания потребителя");
                free(id);
            }
        }
        // Новая команда: удаление производителя в порядке LIFO
        else if (command[0] == 'd') {
            if (prod_count > 0) {
                int removed_number = prod_count;
                if (pthread_cancel(producers[prod_count - 1]) == 0) {
                    pthread_join(producers[prod_count - 1], NULL);
                    prod_count--;
                    printf("Удалён производитель [%d]\n", removed_number);
                } else {
                    printf("Ошибка при удалении производителя.\n");
                }
            } else {
                printf("Нет активных производителей для удаления.\n");
            }
        }
        // Новая команда: удаление потребителя в порядке LIFO
        else if (command[0] == 'f') {
            if (cons_count > 0) {
                int removed_number = cons_count;
                if (pthread_cancel(consumers[cons_count - 1]) == 0) {
                    pthread_join(consumers[cons_count - 1], NULL);
                    cons_count--;
                    printf("Удалён потребитель [%d]\n", removed_number);
                } else {
                    printf("Ошибка при удалении потребителя.\n");
                }
            } else {
                printf("Нет активных потребителей для удаления.\n");
            }
        } else if (command[0] == 'p') {
            print_status(&queue);
        } else if (command[0] == '>') {
            int new_capacity;
            pthread_mutex_lock(&queue.mutex);
            new_capacity = queue.capacity + 1;
            pthread_mutex_unlock(&queue.mutex);
            if (resize_queue(&queue, new_capacity) == 0)
                printf("Очередь увеличена до %d слотов\n", new_capacity);
            else
                printf("Ошибка при увеличении очереди\n");
        } else if (command[0] == '<') {
            pthread_mutex_lock(&queue.mutex);
            if (queue.count != 0) {
                printf("Нельзя уменьшить очередь, пока она не пуста (занято %d элементов)\n", queue.count);
                pthread_mutex_unlock(&queue.mutex);
                continue;
            }
            if (queue.capacity <= 2) {
                printf("Минимальная ёмкость очереди равна 2. Уменьшение невозможно.\n");
                pthread_mutex_unlock(&queue.mutex);
                continue;
            }
            int new_capacity = queue.capacity - 1;
            pthread_mutex_unlock(&queue.mutex);
            if (resize_queue(&queue, new_capacity) == 0)
                printf("Очередь уменьшена до %d слотов\n", new_capacity);
            else
                printf("Ошибка при уменьшении очереди\n");
        } else {
            printf("Неизвестная команда: %s", command);
        }
    }

    // Рассылаем сигнал всем ожидающим потокам для завершения работы
    pthread_mutex_lock(&queue.mutex);
    pthread_cond_broadcast(&queue.not_empty);
    pthread_cond_broadcast(&queue.not_full);
    pthread_mutex_unlock(&queue.mutex);

    for (int i = 0; i < prod_count; i++) {
        pthread_join(producers[i], NULL);
    }
    for (int i = 0; i < cons_count; i++) {
        pthread_join(consumers[i], NULL);
    }

    destroy_queue(&queue);
    printf("Программа завершена.\n");
    return 0;
}
