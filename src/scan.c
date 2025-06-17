#include "scan.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

// Глобальный флаг завершения работы
extern volatile int terminate_flag;

// Отображение части файла с учетом выравнивания
scan_data *scan_file_segment(const char *filename, size_t map_length, off_t map_offset, off_t adjustment, int is_first, int num_blocks) {
    scan_data *data = malloc(sizeof(scan_data));
    if (!data) {
        perror("Ошибка выделения памяти для scan_data");
        return NULL;
    }
    data->memsize = map_length;

    // Проверка флага завершения
    if (terminate_flag) {
        free(data);
        return NULL;
    }

    // Открываем файл для чтения/записи
    data->fd = open(filename, O_RDWR);
    if (data->fd < 0) {
        fprintf(stderr, "Ошибка открытия файла %s: %s\n", filename, strerror(errno));
        free(data);
        return NULL;
    }

    if (terminate_flag) {
        close(data->fd);
        free(data);
        return NULL;
    }

    // Отображение файла в память
    data->map_ptr = mmap(NULL, map_length, PROT_READ | PROT_WRITE, MAP_SHARED, data->fd, map_offset);
    if (data->map_ptr == MAP_FAILED) {
        fprintf(stderr, "Ошибка отображения файла на offset %ld: %s\n", (long)map_offset, strerror(errno));
        close(data->fd);
        free(data);
        return NULL;
    }

    // Проверка флага завершения после mmap()
    if (terminate_flag) {
        munmap(data->map_ptr, map_length);
        close(data->fd);
        free(data);
        return NULL;
    }

    // Извлечение заголовка для первого сегмента
    if (is_first) {
        data->total_records = *((uint64_t *)((char *)data->map_ptr + adjustment));
        data->records = (struct index_s *)((char *)data->map_ptr + adjustment + sizeof(uint64_t));
    } else {
        data->total_records = 0;
        data->records = (struct index_s *)((char *)data->map_ptr + adjustment);
    }

    return data;
}

// Освобождение ресурсов отображения
void free_scan_data(scan_data *data) {
    if (data) {
        if (data->map_ptr && data->map_ptr != MAP_FAILED) {
            munmap(data->map_ptr, data->memsize);
        }
        if (data->fd >= 0) {
            close(data->fd);
        }
        free(data);
    }
}
