#include "sort.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

// Глобальный флаг завершения работы
extern volatile int terminate_flag;

/* Вспомогательная функция сравнения для qsort(). Сравнивает записи по полю time_mark. */
static int cmp_index(const void *a, const void *b) {
    const struct index_s *ia = a;
    const struct index_s *ib = b;
    if (ia->time_mark < ib->time_mark)
        return -1;
    else if (ia->time_mark > ib->time_mark)
        return 1;
    else
        return 0;
}

/* Инициализация структуры sort_data. */
int sort_init(sort_data *sd, struct index_s *records, size_t total_records, size_t block_size, int num_blocks, int num_threads) {
    sd->records = records;
    sd->block_size = block_size;
    sd->num_blocks = num_blocks;
    sd->next_block = num_threads; // Первые блоки сразу распределены между потоками

    if (pthread_mutex_init(&sd->mutex, NULL) != 0) {
        perror("Ошибка инициализации мьютекса");
        return -1;
    }
    if (pthread_barrier_init(&sd->barrier, NULL, num_threads) != 0) {
        perror("Ошибка инициализации барьера");
        return -1;
    }
    return 0;
}

/* Функция сортировки блоков, выполняемая каждым потоком. */
void *sort_blocks(void *arg) {
    sort_thread_arg *targ = (sort_thread_arg *)arg;
    int thread_id = targ->thread_id;
    sort_data *sd = targ->sd;
    int block_index = thread_id;

    // Первоначальная сортировка назначенного блока
    if (block_index < sd->num_blocks && !terminate_flag) {
        struct index_s *block_ptr = sd->records + block_index * sd->block_size;
        qsort(block_ptr, sd->block_size, sizeof(struct index_s), cmp_index);
    }

    // Захват следующих свободных блоков для сортировки
    while (!terminate_flag) {
        pthread_mutex_lock(&sd->mutex);
        if (terminate_flag) {
            pthread_mutex_unlock(&sd->mutex);
            break;
        }

        block_index = sd->next_block;
        if (block_index < sd->num_blocks) {
            sd->next_block++; // резервируем блок
            pthread_mutex_unlock(&sd->mutex);

            if (terminate_flag) break;

            struct index_s *block_ptr = sd->records + block_index * sd->block_size;
            qsort(block_ptr, sd->block_size, sizeof(struct index_s), cmp_index);
        } else {
            pthread_mutex_unlock(&sd->mutex);
            break; // свободных блоков не осталось
        }
    }

    // Синхронизация всех потоков по окончании сортировки
    pthread_barrier_wait(&sd->barrier);
    return NULL;
}
