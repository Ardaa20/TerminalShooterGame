# Derleyici ve bayraklar (-Iinclude kısmı çok önemli, .h dosyalarını bulmasını sağlar)
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

# Kütüphaneler: -lncurses (ncurses için), -lm (matematik işlemleri için) 
LDLIBS = -lncurses -lm

# Klasörler (Tanımlarken sonu DIR ile bitsin)
BUILD_DIR = build
SRC_DIR = src

# Çıktı dosya adı
TARGET = $(BUILD_DIR)/game_demo

# src klasörünün içindeki TÜM .c dosyalarını bul (main.c dahil)
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Varsayılan kural
all: $(BUILD_DIR) $(TARGET)

# Build klasörünü oluştur
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Tüm .c dosyalarını derle ve kütüphaneleri bağla (main.c yi ayrı yazamaya gerek yok ana int main() onda olduğu için onu Başlangıc olarak alıyor)
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDLIBS)

# Temizlik kuralı
clean:
	rm -rf $(BUILD_DIR)

# Derle ve hemen çalıştır
run: all
	./$(TARGET)
