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

        // Определяем сокращенное окружение для дочернего процесса
        char *child_env[] = {
            "SHELL=/bin/bash",
            "HOME=/home/silvarious",
            "HOSTNAME=fedora",
            "LOGNAME=silvarious",
            "LANG=en_US.UTF-8",
            "TERM=xterm-256color",
            "USER=silvarious",
            "LC_COLLATE=C",
            "PATH=/usr/bin:/bin:/usr/sbin:/sbin",
            "CHILD_MODE=star",
            NULL
        };

        // Передаем NULL в аргументах, так как env-файл не требуется
        char *args[] = {child_name, NULL};

        execve(child_exec, args, child_env);
        perror("Ошибка execve");
        exit(1);
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
