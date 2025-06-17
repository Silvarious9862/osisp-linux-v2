#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>

// Структура индексной записи
struct index_s {
    double time_mark; // временная метка (модифицированная юлианская дата)
    uint64_t recno;   // номер записи в БД
};

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_records> <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *endptr;
    uint64_t num_records = strtoull(argv[1], &endptr, 10);
    if (*endptr != '\0' || num_records == 0) {
        fprintf(stderr, "Invalid number of records.\n");
        return EXIT_FAILURE;
    }
    if (num_records % 256 != 0) {
        fprintf(stderr, "Error: number of records (%llu) must be divisible by 256.\n",
                (unsigned long long)num_records);
        return EXIT_FAILURE;
    }

    FILE *fp = fopen(argv[2], "wb");
    if (!fp) {
        perror("Ошибка открытия файла");
        return EXIT_FAILURE;
    }

    // Записываем заголовок: количество записей
    if (fwrite(&num_records, sizeof(num_records), 1, fp) != 1) {
        perror("Ошибка записи заголовка");
        fclose(fp);
        return EXIT_FAILURE;
    }

    // Выделяем память для записей
    struct index_s *records_array = malloc(num_records * sizeof(struct index_s));
    if (!records_array) {
        perror("Ошибка выделения памяти");
        fclose(fp);
        return EXIT_FAILURE;
    }

    // Генератор случайных чисел
    srandom((unsigned int) time(NULL));

    // Вычисляем допустимые границы для временной метки
    time_t now = time(NULL);
    time_t yesterday = now - 86400;
    unsigned long days_since_epoch = yesterday / 86400;
    unsigned long max_int = days_since_epoch + 40587;
    const unsigned long min_int = 15020;
    if (max_int < min_int) {
        fprintf(stderr, "Error: computed max_int (%lu) is less than min_int (%lu).\n", max_int, min_int);
        free(records_array);
        fclose(fp);
        return EXIT_FAILURE;
    }
    unsigned long range = max_int - min_int + 1;

    // Генерация записей
    for (uint64_t i = 0; i < num_records; i++) {
        unsigned long int_part = (unsigned long)(random() % range) + min_int;
        double frac = (double) random() / ((double) RAND_MAX + 1.0);
        records_array[i].time_mark = (double) int_part + frac;
        records_array[i].recno = i + 1;
    }

    // Запись записей в файл
    size_t written = fwrite(records_array, sizeof(struct index_s), num_records, fp);
    if (written != num_records) {
        fprintf(stderr, "Ошибка записи: записано %zu из %llu\n",
                written, (unsigned long long)num_records);
        free(records_array);
        fclose(fp);
        return EXIT_FAILURE;
    }

    free(records_array);
    fclose(fp);
    return EXIT_SUCCESS;
}
