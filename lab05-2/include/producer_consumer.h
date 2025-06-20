#ifndef PRODUCER_CONSUMER_H
#define PRODUCER_CONSUMER_H

#include "common.h"

// Прототипы функций потоков
void *producer_thread(void *arg); // Поток добавления сообщений в очередь
void *consumer_thread(void *arg); // Поток извлечения сообщений из очереди

// Функция вычисления контрольной суммы сообщения
unsigned short calculate_hash(Message *m); // Контрольная сумма для проверки целостности данных

#endif // PRODUCER_CONSUMER_H
