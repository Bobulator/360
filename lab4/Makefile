TARGET = webtest
LIBS = -lm
CC = g++
CFLAGS = -g -Wall

.PHONY: default all clean

default: $(TARGET)
all: default

HOST = bioresearch.byu.edu
PORT = 3000
DIR = /CS360/foo.html
COUNT = 10

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

run: $(TARGET)
	./$(TARGET) $(HOST) $(PORT) $(DIR) -d -c $(COUNT)

valgrind: $(TARGET)
	valgrind --tool=memcheck ./$(TARGET) $(HOST) $(PORT) $(DIR) -d -c $(COUNT)

gdb: $(TARGET)
	gdb --args $(TARGET) $(HOST) $(PORT) $(DIR) -d -c $(COUNT)

clean:
	-rm -f *.o
	-rm -f $(TARGET)
