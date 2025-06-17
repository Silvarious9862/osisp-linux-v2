#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "spawn_children.h"

// Главная функция родительского процесса
int main() {
    int child_count = 0; // Счетчик дочерних процессов
    printf("PID: %d\n", getpid());
    printf("Введите '+', '*', '&' для порождения процесса, 'q' для выхода:\n");

    char input;
    while ((input = getchar()) != EOF) {
        // Запуск дочерних процессов в зависимости от ввода пользователя
        if (input == '+') {
            spawn_child_plus(child_count++, "env"); // Процесс с передачей env-файла
        } else if (input == '*') {
            spawn_child_star(child_count++); // Процесс с сокращенным окружением
        } else if (input == '&') {
            spawn_child_amp(child_count++); // Процесс со стандартным окружением
        } else if (input == 'q') {
            printf("Родительский процесс завершает работу.\n");
            break;
        }

        // Очистка оставшихся символов текущей строки
        if (input != '\n') {
            while (getchar() != '\n');
        }
    }
    return 0;
}
