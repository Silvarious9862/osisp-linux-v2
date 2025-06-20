#ifndef SPAWN_CHILDREN_H
#define SPAWN_CHILDREN_H

// Функции создания дочерних процессов
void spawn_child_plus(int child_number, const char *env_file); // Процесс с передачей env-файла
void spawn_child_star(int child_number); // Процесс с сокращенным окружением
void spawn_child_amp(int child_number);  // Процесс со стандартным окружением

#endif // SPAWN_CHILDREN_H
