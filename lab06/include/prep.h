#ifndef PREP_H
#define PREP_H

#include <pthread.h>

// Аргументы потока
typedef struct thread_arg {
    int thread_id;    // Уникальный идентификатор потока
    int num_threads;  // Общее число потоков
} thread_arg;

// Глобальный барьер синхронизации
extern pthread_barrier_t prep_barrier;
extern int prep_barrier_initialized;

// Инициализация барьера потоков; 0 при успехе, -1 при ошибке
int prep_init_barrier(int num_threads);

// Создание потоков; -1 при ошибке
int prep_create_threads(int num_threads,
                        void *(*thread_func)(void *),
                        pthread_t **threads,
                        thread_arg **targs);

// Ожидание завершения работы всех потоков
void prep_join_threads(int num_threads, pthread_t *threads);

#endif // PREP_H
