#ifndef THREAD_QUEUE_H
#define THREAD_QUEUE_H

#include "common.h"
#include <pthread.h>
#include <semaphore.h>

// Структура динамической очереди сообщений
typedef struct {
    Message *buffer;           // Динамический массив сообщений
    int capacity;              // Текущая ёмкость очереди
    int head;                  // Индекс извлечения
    int tail;                  // Индекс добавления
    int count;                 // Число сообщений в очереди
    int added_count;           // Общий счётчик добавленных сообщений
    int removed_count;         // Общий счётчик извлечённых сообщений
    pthread_mutex_t mutex;     // Мьютекс для защиты очереди
    sem_t sem_free;            // Семафор для свободных слотов
    sem_t sem_full;            // Семафор для заполненных слотов
} ThreadMessageQueue;

// Инициализация очереди (возвращает 0 при успехе, -1 при отказе)
int init_thread_queue(ThreadMessageQueue *q, int capacity);

// Освобождение ресурсов очереди
void destroy_thread_queue(ThreadMessageQueue *q);

// Изменение ёмкости очереди (new_capacity не может быть меньше числа элементов)
int resize_thread_queue(ThreadMessageQueue *q, int new_capacity);

#endif // THREAD_QUEUE_H
