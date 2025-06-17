#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include "common.h"

// Структура динамической (кольцевой) очереди сообщений
typedef struct {
    Message *buffer;          // Динамический массив сообщений
    int capacity;             // Текущая ёмкость очереди
    int count;                // Число сообщений в очереди
    int head;                 // Индекс для извлечения
    int tail;                 // Индекс для вставки
    int added_count;          // Количество добавленных сообщений
    int removed_count;        // Количество извлечённых сообщений
    pthread_mutex_t mutex;    // Мьютекс для защиты очереди
    pthread_cond_t not_empty; // Условная переменная: очередь не пуста
    pthread_cond_t not_full;  // Условная переменная: очередь имеет свободное место
} MessageQueue;

// Функции для работы с очередью
int init_queue(MessageQueue *q, int capacity); // Инициализация очереди
void destroy_queue(MessageQueue *q); // Освобождение ресурсов
void push_message(MessageQueue *q, Message *m); // Добавление сообщения
void pop_message(MessageQueue *q, Message *m); // Извлечение сообщения
int resize_queue(MessageQueue *q, int new_capacity); // Изменение ёмкости очереди
void print_status(MessageQueue *q); // Вывод состояния очереди

#endif // QUEUE_H
