#ifndef INDEX_H
#define INDEX_H

#include <stdint.h>

// Индексная запись (временная метка и номер записи)
struct index_s {
    double time_mark; // Модифицированная юлианская дата
    uint64_t recno;   // Номер записи в БД
};

#endif // INDEX_H
