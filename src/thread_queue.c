#include "thread_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

// Глобальный мьютекс для защиты операций изменения размера очереди
extern pthread_mutex_t resize_mutex;

// Инициализация многопоточной очереди
int init_thread_queue(ThreadMessageQueue *q, int capacity) {
    q->buffer = malloc(capacity * sizeof(Message));
    if (!q->buffer) return -1;
    q->capacity = capacity;
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    q->added_count = 0;
    q->removed_count = 0;
    pthread_mutex_init(&q->mutex, NULL);
    sem_init(&q->sem_free, 0, capacity);
    sem_init(&q->sem_full, 0, 0);
    return 0;
}

// Освобождение памяти очереди
void destroy_thread_queue(ThreadMessageQueue *q) {
    free(q->buffer);
    pthread_mutex_destroy(&q->mutex);
    sem_destroy(&q->sem_free);
    sem_destroy(&q->sem_full);
}

// Изменение размера очереди
int resize_thread_queue(ThreadMessageQueue *q, int new_capacity) {
    pthread_mutex_lock(&resize_mutex);
    pthread_mutex_lock(&q->mutex);

    if (new_capacity < q->count) {
        printf("Невозможно уменьшить очередь ниже количества элементов: %d\n", q->count);
        pthread_mutex_unlock(&q->mutex);
        pthread_mutex_unlock(&resize_mutex);
        return -1;
    }

    int old_capacity = q->capacity;
    Message *new_buffer = realloc(q->buffer, new_capacity * sizeof(Message));
    if (!new_buffer) {
        pthread_mutex_unlock(&q->mutex);
        pthread_mutex_unlock(&resize_mutex);
        return -1;
    }
    q->buffer = new_buffer;

    // Если буфер увеличился — обнуляем новые элементы
    if (new_capacity > old_capacity) {
        memset(q->buffer + old_capacity, 0, (new_capacity - old_capacity) * sizeof(Message));
    }

    q->capacity = new_capacity;

    // Если очередь не начинается с нулевого индекса и в ней что-то есть — выполняем переупорядочивание
    if (q->head != 0 && q->count > 0) {
        Message *temp = malloc(q->capacity * sizeof(Message));
        if (temp) {
            for (int i = 0; i < q->count; i++) {
                temp[i] = q->buffer[(q->head + i) % old_capacity];
            }
            q->head = 0;
            q->tail = q->count;
            for (int i = 0; i < q->count; i++) {
                q->buffer[i] = temp[i];
            }
            free(temp);
        }
    }

    // Коррекция семафоров свободных слотов
    int free_old = old_capacity - q->count;
    int free_new = new_capacity - q->count;
    int delta = free_new - free_old;

    if (delta > 0) {
        for (int i = 0; i < delta; i++) {
            sem_post(&q->sem_free);
        }
    } else if (delta < 0) {
        for (int i = 0; i < -delta; i++) {
            sem_trywait(&q->sem_free);
        }
    }

    printf("Размер очереди изменен: старая ёмкость = %d, новая ёмкость = %d (занято %d)\n",
           old_capacity, new_capacity, q->count);

    pthread_mutex_unlock(&q->mutex);
    pthread_mutex_unlock(&resize_mutex);
    return 0;
}
