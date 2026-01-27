# Makefile for docx-template-render
#
# This Makefile builds the C version of docx-template-render
# using libtct for templating, miniz for ZIP handling, and cJSON for JSON parsing

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
LDFLAGS =
LIBS = -ltct -lm

# Include paths
INCLUDES = -I/usr/local/include -I/usr/include -I.

# Library paths
LIBPATHS = -L/usr/local/lib -L/usr/lib

TARGET = docx-template-render
SOURCES = docx-template-render.c miniz.c cJSON.c

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBPATHS) -o $(TARGET) $(SOURCES) $(LIBS) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o

install: $(TARGET)
	install -D -m 755 $(TARGET) $(DESTDIR)/usr/local/bin/$(TARGET)

uninstall:
	rm -f $(DESTDIR)/usr/local/bin/$(TARGET)
