#include "merge.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

// Глобальный флаг завершения работы
extern volatile int terminate_flag;

// Инициализация структуры merge_data
int merge_init(merge_data *md, struct index_s *records, size_t block_size, int num_blocks, int num_threads) {
    md->records = records;
    md->block_size = block_size;
    md->num_blocks = num_blocks;

    if (pthread_mutex_init(&md->mutex, NULL) != 0) {
        perror("Ошибка инициализации мьютекса (merge)");
        return -1;
    }
    if (pthread_barrier_init(&md->barrier, NULL, num_threads) != 0) {
        perror("Ошибка инициализации барьера (merge)");
        return -1;
    }
    return 0;
}

// Слияние двух отсортированных блоков, результат записывается в block1
void merge_two_sorted_blocks(struct index_s *block1, size_t n1, struct index_s *block2, size_t n2) {
    size_t total = n1 + n2;
    struct index_s *tmp = malloc(total * sizeof(struct index_s));
    if (!tmp) {
        perror("Ошибка выделения памяти для merge_two_sorted_blocks");
        exit(EXIT_FAILURE);
    }

    size_t i = 0, j = 0, k = 0;
    while (i < n1 && j < n2) {
        if (block1[i].time_mark <= block2[j].time_mark)
            tmp[k++] = block1[i++];
        else
            tmp[k++] = block2[j++];
    }
    while (i < n1) {
        tmp[k++] = block1[i++];
    }
    while (j < n2) {
        tmp[k++] = block2[j++];
    }
    memcpy(block1, tmp, total * sizeof(struct index_s));
    free(tmp);
}

// Слияние блоков, выполняемое каждым потоком
void *merge_blocks_phase(void *arg) {
    merge_thread_arg *marg = (merge_thread_arg *) arg;
    int tid = marg->thread_id;
    merge_data *md = marg->md;

    while (md->num_blocks > 1) {
        if (terminate_flag) break;

        int pairs = md->num_blocks / 2;
        if (tid < pairs) {
            int left_index = tid * 2;
            int right_index = tid * 2 + 1;
            struct index_s *left_block = md->records + left_index * md->block_size;
            struct index_s *right_block = md->records + right_index * md->block_size;
            merge_two_sorted_blocks(left_block, md->block_size, right_block, md->block_size);
        }

        pthread_barrier_wait(&md->barrier);
        if (terminate_flag) break;

        if (tid == 0) {
            if (md->num_blocks % 2 == 0)
                md->num_blocks /= 2;
            else
                md->num_blocks = md->num_blocks / 2 + 1;
            md->block_size *= 2;
        }
        pthread_barrier_wait(&md->barrier);
    }

    pthread_barrier_wait(&md->barrier);
    return NULL;
}
