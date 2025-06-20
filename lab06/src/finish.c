#include "finish.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

// Удаление глобального барьера prep_barrier
void finish_cleanup_prep(void) {
    if (prep_barrier_initialized) {
        int res = pthread_barrier_destroy(&prep_barrier);
        if (res != 0) {
            fprintf(stderr, "Ошибка разрушения prep_barrier: %s\n", strerror(res));
        } else {
            printf("prep_barrier успешно разрушен.\n");
        }
        prep_barrier_initialized = 0;
    } else {
        printf("prep_barrier не инициализирован или уже разрушен\n");
    }
}

// Завершение этапа сортировки (удаление барьера и мьютекса)
void finish_cleanup_sort(sort_data *sd) {
    if (pthread_barrier_destroy(&sd->barrier) != 0) {
        perror("Ошибка разрушения барьера в sort_data");
    }
    if (pthread_mutex_destroy(&sd->mutex) != 0) {
        perror("Ошибка разрушения мьютекса в sort_data");
    }
}

// Завершение этапа слияния (удаление барьера и мьютекса)
void finish_cleanup_merge(merge_data *md) {
    if (pthread_barrier_destroy(&md->barrier) != 0) {
        perror("Ошибка разрушения барьера в merge_data");
    }
    if (pthread_mutex_destroy(&md->mutex) != 0) {
        perror("Ошибка разрушения мьютекса в merge_data");
    }
}
