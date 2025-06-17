#ifndef COMMON_H
#define COMMON_H

#define INITIAL_QUEUE_SIZE 10  // Начальный размер очереди
#define MAX_QUEUE_SIZE 256     // Максимальное число сообщений в очереди
#define SHM_KEY 0x1234         // Ключ для разделяемой памяти
#define SEM_KEY 0x5678         // Ключ для семафоров

// Структура сообщения
typedef struct {
    char type;                 // Тип сообщения (1 байт)
    unsigned short hash;       // Контрольная сумма (2 байта)
    unsigned char size;        // Длина данных (0 означает 256 байт)
    char data[256];            // Данные сообщения (выравнены до кратного 4)
} Message;

// Структура кольцевой очереди сообщений
typedef struct {
    int capacity;               // Текущая ёмкость очереди (<= MAX_QUEUE_SIZE)
    Message buffer[MAX_QUEUE_SIZE]; // Буфер для сообщений
    int head;                   // Индекс для извлечения (голова)
    int tail;                   // Индекс для добавления (хвост)
    int added_count;            // Количество добавленных сообщений
    int removed_count;          // Количество извлечённых сообщений
    int free_slots;             // Количество свободных мест (равно capacity - count)
} MessageQueue;

#endif // COMMON_H
