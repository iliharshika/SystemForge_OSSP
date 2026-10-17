CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Iinclude

TARGET = smart_resource_allocation

SOURCES = src/main.c \
          src/banker.c \
          src/resource_manager.c \
          src/resource_request.c \
          src/safety_algorithm.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean