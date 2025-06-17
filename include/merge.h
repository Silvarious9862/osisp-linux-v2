#ifndef MERGE_H
#define MERGE_H

#include <pthread.h>
#include "index.h"  // Структура index_s

// Данные для слияния отсортированных блоков
typedef struct {
    struct index_s *records;   // Массив записей
    size_t block_size;         // Размер блока в записях
    int num_blocks;            // Число оставшихся блоков
    pthread_mutex_t mutex;     // Резерв для динамического распределения (не используется)
    pthread_barrier_t barrier; // Барьер синхронизации фаз слияния
} merge_data;

// Аргументы потока слияния
typedef struct {
    int thread_id;     // Идентификатор потока
    int num_threads;   // Общее число потоков
    merge_data *md;    // Параметры слияния
} merge_thread_arg;

// Инициализация merge_data; 0 при успехе, -1 при ошибке
int merge_init(merge_data *md, struct index_s *records, size_t block_size, int num_blocks, int num_threads);

// Слияние двух отсортированных блоков
void merge_two_sorted_blocks(struct index_s *block1, size_t n1, struct index_s *block2, size_t n2);

// Функция потока для слияния блоков
void *merge_blocks_phase(void *arg);

#endif // MERGE_H
