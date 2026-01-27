# Makefile for docx-template-render
#
# This Makefile builds the C version of docx-template-render
# using libtct and doctxt libraries

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
LDFLAGS =
LIBS = -ltct -ldoctxt

# Include paths for libtct and doctxt headers
INCLUDES = -I/usr/local/include -I/usr/include

# Library paths
LIBPATHS = -L/usr/local/lib -L/usr/lib

TARGET = docx-template-render
SOURCE = docx-template-render.c

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBPATHS) -o $(TARGET) $(SOURCE) $(LIBS) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o

install: $(TARGET)
	install -D -m 755 $(TARGET) $(DESTDIR)/usr/local/bin/$(TARGET)

uninstall:
	rm -f $(DESTDIR)/usr/local/bin/$(TARGET)
