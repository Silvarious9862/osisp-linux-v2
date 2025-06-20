#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "spawn_children.h"

// Глобальный указатель на окружение родительского процесса
extern char **environ;

// Глобальная переменная для сокращённого окружения, которая будет использована при запуске дочерних процессов
char **reduced_env;

// Функция сравнения для сортировки строк с использованием strcmp
int cmp_env(const void *a, const void *b) {
    const char * const *s1 = a;
    const char * const *s2 = b;
    return strcmp(*s1, *s2);
}

// Копирование и сортировка переменных окружения родителя
void print_sorted_environment(void) {
    // Подсчитываем количество переменных
    int count = 0;
    for (char **env = environ; *env != NULL; env++) {
        count++;
    }

    // Копируем указатели в новый массив
    char **env_copy = malloc(count * sizeof(char *));
    if (!env_copy) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < count; i++) {
        env_copy[i] = environ[i];
    }

    // Устанавливаем локаль для сортировки и сортируем
    setlocale(LC_COLLATE, "C");
    qsort(env_copy, count, sizeof(char *), cmp_env);

    // Выводим отсортированное окружение
    for (int i = 0; i < count; i++) {
        printf("%s\n", env_copy[i]);
    }
    free(env_copy);
}

// Функция формирования сокращённого окружения для дочернего процесса.
char **create_reduced_env(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Ошибка открытия файла env");
        exit(EXIT_FAILURE);
    }
    size_t capacity = 10, count = 0;
    char **env_arr = malloc(capacity * sizeof(char *));
    if (!env_arr) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        // Удаляем символы конца строки (\n, \r)
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0')
            continue;
        // Получаем значение переменной из родительского окружения
        char *value = getenv(line);
        if (value == NULL)
            continue;
        // Выделяем память для строки "ИМЯ=значение"
        size_t len = strlen(line) + 1 + strlen(value) + 1; // имя + '=' + значение + '\0'
        char *entry = malloc(len);
        if (!entry) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        snprintf(entry, len, "%s=%s", line, value);
        // Расширяем массив при необходимости
        if (count >= capacity) {
            capacity *= 2;
            char **temp = realloc(env_arr, capacity * sizeof(char *));
            if (!temp) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
            env_arr = temp;
        }
        env_arr[count++] = entry;
    }
    fclose(fp);
    // Добавляем завершающий NULL, как требуется для execve()
    env_arr = realloc(env_arr, (count + 1) * sizeof(char *));
    if (!env_arr) {
        perror("realloc");
        exit(EXIT_FAILURE);
    }
    env_arr[count] = NULL;
    return env_arr;
}

void free_reduced_env(char **env_arr) {
    if (!env_arr)
        return;
    for (size_t i = 0; env_arr[i] != NULL; i++) {
        free(env_arr[i]);
    }
    free(env_arr);
}

int main(void) {
    if (getenv("LC_COLLATE") == NULL) {
        setenv("LC_COLLATE", "C", 1);
    }

    int child_count = 0;

    // 1. Вывод отсортированного окружения родительского процесса (в локали "C")
    printf("Отсортированное окружение родительского процесса (LC_COLLATE=C):\n");
    print_sorted_environment();
    printf("\n");

    // 2. Формирование сокращённого окружения для дочернего процесса из файла "env"
    reduced_env = create_reduced_env("env");
    printf("Сформированная сокращенная среда для дочернего процесса:\n");
    for (int i = 0; reduced_env[i] != NULL; i++) {
        printf("%s\n", reduced_env[i]);
    }
    printf("\n");

    // 3. Вывод PID родительского процесса и приглашение для ввода команд
    printf("PID: %d\n", getpid());
    printf("Введите '+', '*', '&' для порождения процесса, 'q' для выхода:\n");

    char input;
    while ((input = getchar()) != EOF) {
        if (input == '+') {
            // Функция spawn_child_plus() передаёт в дочерний процесс путь к файлу "env"
            spawn_child_plus(child_count++, "env");
        } else if (input == '*') {
            spawn_child_star(child_count++);
        } else if (input == '&') {
            spawn_child_amp(child_count++);
        } else if (input == 'q') {
            printf("Родительский процесс завершает работу.\n");
            break;
        }
        // Очистка оставшихся символов в строке
        if (input != '\n') {
            while (getchar() != '\n');
        }
    }

    free_reduced_env(reduced_env);
    return 0;
}
