// main.c
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

volatile int terminate_flag = 0;
ThreadMessageQueue queue;
pthread_mutex_t resize_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int pause_processing = 0;

// Функция вывода текущего состояния очереди
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

// Обработчик SIGINT (Ctrl-C)
static void sigint_handler(int sig) {
    (void)sig;  // подавляем предупреждение о неиспользуемом параметре
    printf("\nПолучен SIGINT. Запускается завершение работы...\n");
    terminate_flag = 1;
}

// Поток для периодической проверки дедлоков
void* deadlock_monitor(void* arg) {
    while (!terminate_flag) {
        // Например, можно добавить проверку состояния очереди
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
    sa.sa_flags = 0; // для перезапуска некоторых системных вызовов
    if(sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Ошибка sigaction");
        exit(1);
    }
    
    srand(time(NULL));
    if (init_thread_queue(&queue, 10) != 0) {
        perror("Ошибка инициализации очереди");
        return 1;
    }
    
    pthread_t producers[MAX_THREADS], consumers[MAX_THREADS];
    int prod_count = 0, cons_count = 0;
    char command[10];

    printf("Команды:\n");
    printf("  + : добавить поток-производитель\n");
    printf("  - : добавить поток-потребитель\n");
    printf("  > : увеличить размер очереди\n");
    printf("  < : уменьшить размер очереди\n");
    printf("  p : вывести состояние очереди\n");
    printf("  q : завершение работы\n");
    
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
        } else if (command[0] == 'p') {
            print_status();
        } else if (command[0] == '>') {
            pthread_mutex_lock(&queue.mutex);
            int new_capacity = queue.capacity + 1;
            pthread_mutex_unlock(&queue.mutex);
            if (resize_thread_queue(&queue, new_capacity) == 0)
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
            
            pause_processing = 1;
            sleep(1);  // небольшая задержка, чтобы текущие итерации завершились
            
            if (resize_thread_queue(&queue, new_capacity) == 0)
                printf("Очередь уменьшена до %d слотов\n", new_capacity);
            else
                printf("Ошибка при уменьшении очереди\n");
            pause_processing = 0;
        } else {
            printf("Неизвестная команда: %s", command);
        }
    }
    
    // Дожидаемся завершения всех потоков
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
