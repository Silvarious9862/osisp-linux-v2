#ifndef SCAN_H
#define SCAN_H

#include <stdint.h>
#include <stddef.h>
#include "index.h"  // Структура index_s
#include <sys/stat.h>

// Информация об отображении файла
typedef struct {
    int fd;                 // Дескриптор файла
    void *map_ptr;          // Адрес отображённой области
    size_t memsize;         // Размер отображённой области
    uint64_t total_records; // Количество записей (из заголовка)
    struct index_s *records; // Массив записей (начало после заголовка)
} scan_data;

// Отображение файла в память
scan_data *scan_file(const char *filename, size_t memsize);

scan_data *scan_file_segment(const char *filename, size_t map_length, off_t map_offset, off_t adjustment, int is_first, int num_blocks);

// Освобождение ресурсов (отмена отображения и закрытие файла)
void free_scan_data(scan_data *data);

#endif // SCAN_H
