TARGET = server
LIBS = -lm
CC = g++
CFLAGS = -g -Wall

.PHONY: default all clean

default: $(TARGET)
all: default

PORT = 3000
DIR = /home/bitnami/htdocs/CS360/lab3/testDir

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

run: server
	./server $(PORT) $(DIR)

valgrind: server
	valgrind --tool=memcheck ./server $(PORT) $(DIR)

gdb: server
	gdb --args server $(PORT) $(DIR)

clean:
	-rm -f *.o
	-rm -f $(TARGET)
