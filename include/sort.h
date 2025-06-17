#ifndef SORT_H
#define SORT_H

#include <stddef.h>
#include <pthread.h>
#include "index.h"  // Структура index_s

// Параметры сортировки блоков
typedef struct {
    struct index_s *records;   // Массив записей для сортировки
    size_t block_size;         // Число записей в блоке
    int num_blocks;            // Общее число блоков
    int next_block;            // Следующий свободный блок
    pthread_mutex_t mutex;     // Мьютекс для next_block
    pthread_barrier_t barrier; // Барьер синхронизации потоков
} sort_data;

// Аргумент потока сортировки
typedef struct {
    int thread_id;     // Идентификатор потока
    sort_data *sd;     // Параметры сортировки
} sort_thread_arg;

// Инициализация структуры sort_data; 0 при успехе, -1 при ошибке
int sort_init(sort_data *sd, struct index_s *records, size_t total_records, size_t block_size, int num_blocks, int num_threads);

// Функция сортировки блоков, выполняемая потоком
void *sort_blocks(void *arg);

#endif // SORT_H
