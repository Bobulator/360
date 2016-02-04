TARGET = server
LIBS = -lm -lpthread
CC = g++
CFLAGS = -g -Wall

.PHONY: default all clean

default: $(TARGET)
all: default

PORT = 3000
THREADS = 10
DIR = /home/cbacon/Documents/CS360/lab3/testDir

OBJECTS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))
HEADERS = $(wildcard *.h)

%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

run: server
	./server $(PORT) $(THREADS) $(DIR)

valgrind: server
	valgrind --tool=memcheck --leak-check=full ./server $(PORT) $(THREADS) $(DIR)

gdb: server
	gdb --args server $(PORT) $(THREADS) $(DIR)

clean:
	-rm -f *.o
	-rm -f $(TARGET)
