#ifndef FINISH_H
#define FINISH_H

#include "prep.h"   // Глобальный барьер prep_barrier
#include "sort.h"   // Структура sort_data
#include "merge.h"  // Структура merge_data

extern int prep_barrier_initialized;

// Удаление глобального барьера prep_barrier
void finish_cleanup_prep(void);

// Удаление барьера и мьютекса в sort_data
void finish_cleanup_sort(sort_data *sd);

// Удаление барьера и мьютекса в merge_data
void finish_cleanup_merge(merge_data *md);

#endif // FINISH_H
