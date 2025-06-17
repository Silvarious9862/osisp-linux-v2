#ifndef COMMON_H
#define COMMON_H

// Размер буфера для данных сообщения
#define DATA_SIZE 256  // Максимальный размер данных сообщения

// Начальная ёмкость очереди
#define INITIAL_CAPACITY 10  // Количество слотов в очереди при инициализации

// Структура сообщения
typedef struct {
    char type;                 // Тип сообщения (один символ)
    unsigned short hash;       // Контрольная сумма сообщения
    unsigned char size;        // Если 0, то означает 256 байт
    char data[DATA_SIZE];      // Данные сообщения (максимальная длина 256)
} Message;

#endif // COMMON_H
