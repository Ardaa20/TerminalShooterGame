# Derleyici ve bayraklar
CC = gcc
CFLAGS = -Wall -Wextra

# Kütüphaneler: -lncurses (ncurses için), -lm (matematik işlemleri için)
LDLIBS = -lncurses -lm

# Çıktı klasörü ve dosya adı
BUILD_DIR = build
TARGET = $(BUILD_DIR)/doom_game

# Varsayılan kural
all: $(BUILD_DIR) $(TARGET)

# Build klasörünü oluştur
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# main.c'yi derle ve kütüphaneleri bağla
$(TARGET): main.c
	$(CC) $(CFLAGS) main.c -o $(TARGET) $(LDLIBS)

# Temizlik kuralı
clean:
	rm -rf $(BUILD_DIR)

# Derle ve hemen çalıştır
run: all
	./$(TARGET)
