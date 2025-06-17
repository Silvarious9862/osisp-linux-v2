BUILD ?= debug

# Определение каталогов
SRC_DIR := ./src
INC_DIR := ./include
OUT_DIR := ./build/$(BUILD)

# Бинарные файлы
TARGET_PARENT := $(OUT_DIR)/parent
TARGET_CHILD  := $(OUT_DIR)/child

# Исходные файлы
SRCS_PARENT := $(SRC_DIR)/parent.c
SRCS_CHILD  := $(SRC_DIR)/child.c

# Компилятор
CC := gcc

# Общие флаги компиляции
COMMON_CFLAGS := -D_GNU_SOURCE -D_POSIX_C_SOURCE=200112L -W -Wall -Wextra -std=c11 -pedantic -Wno-unused-parameter -Wno-unused-variable -I$(INC_DIR)

# Определение флагов для режима отладки и релиза
ifeq ($(BUILD), debug)
	CFLAGS := $(COMMON_CFLAGS) -g -O0 -fsanitize=address -fno-omit-frame-pointer
else ifeq ($(BUILD), release)
	CFLAGS := $(COMMON_CFLAGS) -O2
else
	$(error Unknown build type: $(BUILD))
endif

.PHONY: all clean parent child debug release

# Сборка всех бинарных файлов
all: $(TARGET_PARENT) $(TARGET_CHILD)

# Сборка родительского процесса
$(TARGET_PARENT): $(SRCS_PARENT)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $(SRCS_PARENT)

# Сборка дочернего процесса
$(TARGET_CHILD): $(SRCS_CHILD)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $(SRCS_CHILD)

# Отдельные цели для сборки
parent: $(TARGET_PARENT)
child: $(TARGET_CHILD)

# Отладочная сборка
debug:
	$(MAKE) BUILD=debug all

# Оптимизированная сборка
release:
	$(MAKE) BUILD=release all

# Очистка сборки
clean:
	rm -rf ./build
