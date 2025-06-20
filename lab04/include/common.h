#ifndef COMMON_H
#define COMMON_H

#include <sys/ipc.h>

#define QUEUE_SIZE 10  // Размер кольцевого буфера
#define SHM_KEY 0x1234 // Ключ для разделяемой памяти
#define SEM_KEY 0x5678 // Ключ для семафоров

// Структура сообщения в очереди
typedef struct {
    char type;                 // Тип сообщения
    unsigned short hash;       // Контрольная сумма
    unsigned char size;        // Размер данных
    char data[256];            // Данные сообщения
} Message;

// Структура кольцевого буфера сообщений
typedef struct {
    Message buffer[QUEUE_SIZE]; // Буфер сообщений
    int head;                   // Индекс извлечения
    int tail;                   // Индекс добавления
    int added_count;            // Общее количество добавленных сообщений
    int removed_count;          // Общее количество извлечённых сообщений
    int free_slots;             // Количество свободных слотов в буфере
} MessageQueue;

#endif // COMMON_H
