#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include "common.h"

#define MAX_PRODUCERS 100
#define MAX_CONSUMERS 100

// Глобальные переменные для управления процессами и IPC-ресурсами
volatile sig_atomic_t terminate_flag = 0;
pid_t producer_pids[MAX_PRODUCERS];
pid_t consumer_pids[MAX_CONSUMERS];
int producer_count = 0;
int consumer_count = 0;

MessageQueue* queue;
int shm_id;
int sem_id;

// Проверка возможных дедлоков в системе
void check_deadlock() {
    if (producer_count == 0 && queue->free_slots == QUEUE_SIZE) {
        printf("---Нет активных производителей! Нужно добавить производителей.---\n");
    }
    if (consumer_count == 0 && queue->free_slots == 0) {
        printf("---Нет активных потребителей! Нужно добавить потребителей.---\n");
    }
}

// Поток для периодической проверки дедлоков. Цикл завершается, как только terminate_flag установлен.
void* deadlock_monitor(void* arg) {
    while (!terminate_flag) {
        check_deadlock();
        sleep(5);
    }
    return NULL;
}

// Инициализация общей памяти и семафоров
void init_resources() {
    shm_id = shmget(SHM_KEY, sizeof(MessageQueue), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("Ошибка при создании общей памяти");
        exit(1);
    }
    queue = (MessageQueue*) shmat(shm_id, NULL, 0);
    if (queue == (void*) -1) {
        perror("Ошибка подключения к общей памяти");
        exit(1);
    }
    memset(queue, 0, sizeof(MessageQueue));
    queue->free_slots = QUEUE_SIZE;

    sem_id = semget(SEM_KEY, 3, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("Ошибка при создании семафоров");
        exit(1);
    }

    // Инициализация семафоров: [0] — свободные слоты, [1] — занятые слоты, [2] — мьютекс
    semctl(sem_id, 0, SETVAL, QUEUE_SIZE);
    semctl(sem_id, 1, SETVAL, 0);
    semctl(sem_id, 2, SETVAL, 1);
}

// Очистка IPC-ресурсов и завершение дочерних процессов
void cleanup_resources() {
    // Перед очисткой убеждаемся, что флаг завершения уже выставлен
    terminate_flag = 1;
    for (int i = 0; i < producer_count; i++) {
        kill(producer_pids[i], SIGTERM);
    }
    for (int i = 0; i < consumer_count; i++) {
        kill(consumer_pids[i], SIGTERM);
    }
    shmdt(queue);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);
}

// Вывод текущего состояния очереди
void print_status() {
    printf("Состояние очереди:\n");
    printf("Добавлено сообщений: %d\n", queue->added_count);
    printf("Извлечено сообщений: %d\n", queue->removed_count);
    printf("Свободные места: %d\n", queue->free_slots);
    printf("Занятые места: %d\n", QUEUE_SIZE - queue->free_slots);
    printf("Производителей: %d, Потребителей: %d\n", producer_count, consumer_count);
}

// Обработчик SIGINT теперь просто устанавливает флаг завершения.
static void sigint_handler(int sig) {
    (void)sig;
    printf("\nПолучен SIGINT. Инициируется завершение работы...\n");
    terminate_flag = 1;
}

// Обработчик SIGCHLD для сбора завершённых дочерних процессов
void sigchld_handler(int signo) {
    (void) signo;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

// Обработка команд пользователя. Цикл завершается, если получен сигнал завершения (terminate_flag) или команда 'q'.
void handle_commands() {
    char command[10];
    printf("Команды: '+' – запустить производителя, '-' – запустить потребителя, \n"
           "         '>' – удалить производителя, '<' – удалить потребителя,\n"
           "         'p' – показать статус, 'q' – выйти.\n");
    while (!terminate_flag) {
        if (fgets(command, sizeof(command), stdin) != NULL) {
            switch (command[0]) {
                case '+': {
                    pid_t pid = fork();
                    if (pid == 0) {
                        execl("./producer", "./producer", NULL);
                        perror("Ошибка при запуске производителя");
                        exit(1);
                    } else {
                        producer_pids[producer_count++] = pid;
                    }
                    break;
                }
                case '-': {
                    pid_t pid = fork();
                    if (pid == 0) {
                        execl("./consumer", "./consumer", NULL);
                        perror("Ошибка при запуске потребителя");
                        exit(1);
                    } else {
                        consumer_pids[consumer_count++] = pid;
                    }
                    break;
                }
                case '>': {
                    if (producer_count > 0) {
                        pid_t pid = producer_pids[producer_count - 1];
                        kill(pid, SIGTERM);
                        producer_count--;
                        printf("Удален последний производитель (PID: %d).\n", pid);
                    } else {
                        printf("Нет активных производителей для удаления.\n");
                    }
                    break;
                }
                case '<': {
                    if (consumer_count > 0) {
                        pid_t pid = consumer_pids[consumer_count - 1];
                        kill(pid, SIGTERM);
                        consumer_count--;
                        printf("Удален последний потребитель (PID: %d).\n", pid);
                    } else {
                        printf("Нет активных потребителей для удаления.\n");
                    }
                    break;
                }
                case 'p': {
                    print_status();
                    break;
                }
                case 'q': {
                    printf("Запрошено завершение программы.\n");
                    terminate_flag = 1;
                    break;
                }
                default:
                    printf("Неизвестная команда: %s", command);
                    break;
            }
        } else {
            if (terminate_flag)
                break;
            printf("Ошибка ввода команды. Попробуйте снова.\n");
        }
    }
}

int main() {
    // Регистрация обработчика SIGINT
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // Регистрация обработчика SIGCHLD для избежания зомби-процессов
    struct sigaction sa_chld;
    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa_chld, NULL) == -1) {
        perror("sigaction(SIGCHLD)");
        exit(1);
    }

    printf("Запуск основного процесса.\n");
    init_resources();

    // Запуск потока для мониторинга дедлоков; он завершится, когда terminate_flag станет истинным.
    pthread_t monitor_thread;
    if (pthread_create(&monitor_thread, NULL, deadlock_monitor, NULL) != 0) {
        perror("pthread_create");
        exit(1);
    }

    // Обработка команд пользователя;
    // цикл завершится, если получен сигнал завершения (SIGINT или команда 'q')
    handle_commands();

    // Ожидание завершения потока мониторинга
    pthread_join(monitor_thread, NULL);

    // Очистка ресурсов перед выходом
    cleanup_resources();
    printf("Ресурсы очищены. Завершается программа.\n");
    return 0;
}
