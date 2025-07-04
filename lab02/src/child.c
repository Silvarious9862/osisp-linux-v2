#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char **environ; // Глобальная переменная окружения

// Обработка режима '+': загрузка переменных окружения из файла и использование getenv()
void handle_plus_mode(const char *env_file) {
    printf("\n[Режим '+']: Чтение переменных из файла %s и использование getenv()\n", env_file);

    FILE *file = fopen(env_file, "r");
    if (!file) {
        perror("Ошибка открытия файла");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0'; // Удаление символа новой строки
        char *value = getenv(line);
        if (value) {
            printf("%s=%s\n", line, value);
        } else {
            printf("%s=<не найдено>\n", line);
        }
    }

    fclose(file);
}

// Обработка режима '*': вывод переданного массива envp
void handle_star_mode(char *envp[]) {
    printf("\n[Режим '*']: Использование параметра envp\n");

    for (char **env = envp; *env != NULL; env++) {
        printf("%s\n", *env);
    }
}

// Обработка режима '&': вывод всех переменных окружения через глобальный массив environ
void handle_amp_mode() {
    printf("\n[Режим '&']: Использование глобальной переменной environ\n");

    for (char **env = environ; *env != NULL; env++) {
        printf("%s\n", *env);
    }
}

// Главная функция программы
int main(int argc, char *argv[], char *envp[]) {
    // Вывод базовой информации о процессе
    printf("Имя программы: %s\n", argv[0]);
    printf("PID: %d\n", getpid());
    printf("PPID: %d\n", getppid());

    // Определение режима работы:
    // - Если передан аргумент, используется режим '+'
    // - Если переменная CHILD_MODE установлена, используется режим '*'
    // - В остальных случаях применяется режим '&'
    if (argc > 1) {
        handle_plus_mode(argv[1]);
    }
    else if (getenv("CHILD_MODE") != NULL) {
        handle_star_mode(envp);
    }
    else {
        handle_amp_mode();
    }

    printf("\nДочерний процесс завершен.\n");
    return 0;
}
