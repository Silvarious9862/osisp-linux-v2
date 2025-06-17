#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "prep.h"

// Глобальный флаг завершения работы
extern volatile int terminate_flag;

// Барьер для синхронизации потоков
pthread_barrier_t prep_barrier;
int prep_barrier_initialized = 0;

// Инициализация барьера; 0 при успехе, -1 при ошибке
int prep_init_barrier(int num_threads) {
    int ret = pthread_barrier_init(&prep_barrier, NULL, num_threads);
    if (ret != 0) {
        fprintf(stderr, "Ошибка инициализации барьера: %s\n", strerror(ret));
        return -1;
    }
    prep_barrier_initialized = 1;
    return 0;
}

// Создание потоков
int prep_create_threads(int num_threads,
                        void *(*thread_func)(void *),
                        pthread_t **threads,
                        thread_arg **targs) {
    if (terminate_flag) {
        fprintf(stderr, "Прерывание: terminate_flag установлен. Потоки не будут созданы.\n");
        return -1;
    }

    *threads = malloc(num_threads * sizeof(pthread_t));
    if (!*threads) {
        perror("Ошибка выделения памяти для threads");
        return -1;
    }

    *targs = malloc(num_threads * sizeof(thread_arg));
    if (!*targs) {
        perror("Ошибка выделения памяти для thread args");
        free(*threads);
        return -1;
    }

    for (int i = 0; i < num_threads; i++) {
        if (terminate_flag) {
            fprintf(stderr, "Прерывание при создании потоков: terminate_flag установлен.\n");
            free(*threads);
            free(*targs);
            return -1;
        }
        (*targs)[i].thread_id = i;
        (*targs)[i].num_threads = num_threads;
        int ret = pthread_create(&(*threads)[i], NULL, thread_func, &(*targs)[i]);
        if (ret != 0) {
            fprintf(stderr, "Ошибка создания потока %d: %s\n", i, strerror(ret));
            free(*threads);
            free(*targs);
            return -1;
        }
    }
    return 0;
}

// Ожидание завершения всех потоков
void prep_join_threads(int num_threads, pthread_t *threads) {
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}
