#include "parent.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

// Массив дочерних процессов и их счётчик
pid_t children[MAX_CHILDREN];
int child_count = 0;

// Создание нового дочернего процесса
void add_child(void) {
    if (child_count >= MAX_CHILDREN) {
        printf("Достигнуто максимальное количество дочерних процессов.\n");
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Ошибка при fork");
        exit(1);
    } else if (pid == 0) {
        execl("./child", "./child", NULL); // Запуск дочернего процесса
        perror("Ошибка при запуске дочернего процесса");
        exit(1);
    } else {
        children[child_count++] = pid;
        printf("Создан дочерний процесс с PID %d.\n", pid);
    }
}

// Завершение последнего дочернего процесса
void remove_last_child(void) {
    if (child_count == 0) {
        printf("Нет активных дочерних процессов для завершения.\n");
        return;
    }

    pid_t pid = children[--child_count];
    kill(pid, SIGTERM);
    waitpid(pid, NULL, 0);
    printf("Завершен дочерний процесс с PID %d. Осталось процессов: %d.\n", pid, child_count);
}

// Вывод списка активных процессов
void list_processes(void) {
    printf("Родительский процесс PID %d.\n", getpid());
    for (int i = 0; i < child_count; i++) {
        printf("Дочерний процесс PID %d.\n", children[i]);
    }
}

// Завершение всех дочерних процессов
void kill_all_children(void) {
    for (int i = 0; i < child_count; i++) {
        kill(children[i], SIGTERM);
        waitpid(children[i], NULL, 0);
        printf("Завершен дочерний процесс с PID %d.\n", children[i]);
    }
    child_count = 0;
}

// Управление вводом команд
void handle_input(void) {
    char command;
    while (1) {
        command = getchar();
        switch (command) {
            case '+':
                add_child();
                break;
            case '-':
                remove_last_child();
                break;
            case 'l':
                list_processes();
                break;
            case 'k':
                kill_all_children();
                break;
            case 'o': 
                for (int i = 0; i < child_count; i++) {
                    kill(children[i], SIGUSR2);
                    usleep(50000); // Упорядочивание вывода (50 мс)
                }
                break;
            case 'q':
                kill_all_children();
                printf("Родительский процесс завершен.\n");
                exit(0);
            default:
                break;
        }
    }
}

// Главная функция программы
int main() {
    printf("Введите команду:\n"
           "  '+' для создания дочернего процесса,\n"
           "  '-' для удаления последнего дочернего процесса,\n"
           "  'l' для вывода списка процессов,\n"
           "  'o' для упорядоченного вывода статистики,\n"
           "  'k' для завершения всех дочерних процессов,\n"
           "  'q' для выхода.\n");
    handle_input();
    return 0;
}
