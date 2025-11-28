CC = gcc
CFLAGS = -Wall -g -fsanitize=address
TARGET = huffman

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) main.c -o $(TARGET)

clean:
	rm -f $(TARGET)
	rm -f *.o

