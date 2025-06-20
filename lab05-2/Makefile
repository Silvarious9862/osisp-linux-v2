BUILD ?= debug

# Определение каталогов
SRC_DIR := ./src
INC_DIR := ./include
OUT_DIR := ./build/$(BUILD)

# Бинарный файл для многопоточной версии
TARGET := $(OUT_DIR)/threads

# Исходные файлы
SRCS := $(SRC_DIR)/main.c $(SRC_DIR)/producer.c $(SRC_DIR)/consumer.c $(SRC_DIR)/queue.c

# Компилятор
CC := gcc

# Общие флаги компиляции
COMMON_CFLAGS := -D_GNU_SOURCE -D_POSIX_C_SOURCE=200112L -W -Wall -Wextra -std=c11 -pedantic -I$(INC_DIR)

# Определение флагов для режимов сборки
ifeq ($(BUILD), debug)
	CFLAGS := $(COMMON_CFLAGS) -g -O0 -fsanitize=address -fno-omit-frame-pointer
else ifeq ($(BUILD), release)
	CFLAGS := $(COMMON_CFLAGS) -O2
else
	$(error Unknown build type: $(BUILD))
endif

.PHONY: all clean debug release

# Сборка бинарного файла
all: $(TARGET)

$(TARGET): $(SRCS)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -pthread -o $@ $(SRCS)

# Отладочная сборка
debug:
	$(MAKE) BUILD=debug all

# Оптимизированная сборка
release:
	$(MAKE) BUILD=release all

# Очистка сборки
clean:
	rm -rf ./build
