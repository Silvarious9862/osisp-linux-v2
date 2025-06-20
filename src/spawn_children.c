#include "spawn_children.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

// Глобальная переменная окружения
extern char **environ;

// Создание дочернего процесса в режиме '+'
void spawn_child_plus(int child_number, const char *env_file) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("Ошибка fork");
        exit(1);
    } else if (pid == 0) {
        // Формируем имя дочернего процесса
        char child_name[20];
        snprintf(child_name, sizeof(child_name), "child_%02d", child_number);

        // Получаем путь к исполняемому файлу дочернего процесса
        char *child_path = getenv("CHILD_PATH");
        if (!child_path) {
            fprintf(stderr, "Ошибка: переменная окружения CHILD_PATH не задана.\n");
            exit(1);
        }

        char child_exec[512];
        snprintf(child_exec, sizeof(child_exec), "%s/child", child_path);

        // Передача пути к env-файлу в аргументах
        char *args[] = {child_name, (char *)env_file, NULL};

        execve(child_exec, args, environ);
        perror("Ошибка execve");
        exit(1);
    } else {
        printf("Запущен дочерний процесс %d с PID %d (режим '+')\n", child_number, pid);
    }
}

// Создание дочернего процесса в режиме '*'
void spawn_child_star(int child_number) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("Ошибка fork");
        exit(1);
    } else if (pid == 0) {
        // Формируем имя дочернего процесса, например, "child_01"
        char child_name[20];
        snprintf(child_name, sizeof(child_name), "child_%02d", child_number);

        // Получаем путь к исполняемому файлу дочернего процесса из переменной окружения CHILD_PATH
        char *child_path = getenv("CHILD_PATH");
        if (!child_path) {
            fprintf(stderr, "Ошибка: переменная окружения CHILD_PATH не задана.\n");
            exit(1);
        }
        char child_exec[512];
        snprintf(child_exec, sizeof(child_exec), "%s/child", child_path);

        // Создаем новый массив переменных окружения для дочернего процесса,
        // копируя все указатели из reduced_env и добавляя переменную "CHILD_MODE=star".
        extern char **reduced_env;
        int count = 0;
        while (reduced_env[count] != NULL) {
            count++;
        }
        // Выделяем память под новый массив: имеем count переменных, плюс одна новая и завершающий NULL.
        char **new_env = malloc((count + 2) * sizeof(char *));
        if (!new_env) {
            perror("malloc");
            exit(1);
        }

        for (int i = 0; i < count; i++) {
            new_env[i] = reduced_env[i];
        }
        new_env[count] = "CHILD_MODE=star";
        new_env[count + 1] = NULL;

        // Формируем аргументы для дочернего процесса:
        // argv[0] – имя дочернего процесса, никаких дополнительных параметров не передаем.
        char *args[] = { child_name, NULL };

        // Запускаем дочерний процесс, передавая сформированный массив окружения.
        if (execve(child_exec, args, new_env) < 0) {
            perror("Ошибка execve");
            exit(1);
        }
    } else {
        printf("Запущен дочерний процесс %d с PID %d (режим '*')\n", child_number, pid);
    }
}


// Создание дочернего процесса в режиме '&'
void spawn_child_amp(int child_number) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("Ошибка fork");
        exit(1);
    } else if (pid == 0) {
        // Формируем имя дочернего процесса
        char child_name[20];
        snprintf(child_name, sizeof(child_name), "child_%02d", child_number);

        // Получаем путь к исполняемому файлу дочернего процесса
        char *child_path = getenv("CHILD_PATH");
        if (!child_path) {
            fprintf(stderr, "Ошибка: переменная окружения CHILD_PATH не задана.\n");
            exit(1);
        }

        char child_exec[512];
        snprintf(child_exec, sizeof(child_exec), "%s/child", child_path);

        // Используем стандартное окружение без CHILD_MODE
        char *args[] = {child_name, NULL};

        execve(child_exec, args, environ);
        perror("Ошибка execve");
        exit(1);
    } else {
        printf("Запущен дочерний процесс %d с PID %d (режим '&')\n", child_number, pid);
    }
}
