#ifndef IPC_H
#define IPC_H

#include "common.h"

// Операция над семафором
void semaphore_op(int sem_id, int sem_num, int op);

// Вычисление контрольной суммы сообщения
unsigned short calculate_hash(Message* message);

// Проверка корректности контрольной суммы
int verify_hash(Message* message);

#endif // IPC_H
