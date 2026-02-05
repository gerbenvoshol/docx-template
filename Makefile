# Makefile for docx-template-render and pdf-template-render
#
# This Makefile builds both C versions:
# - docx-template-render: using libtct, miniz for ZIP handling, and mjson for JSON parsing
# - pdf-template-render: using libtct, mjson for JSON parsing, and pdfgen for PDF generation

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
LDFLAGS =
LIBS = -lm

# Include paths
INCLUDES = -I.

TARGET_DOCX = docx-template-render
SOURCES_DOCX = docx-template-render.c miniz.c mjson.c libtct.c

TARGET_PDF = pdf-template-render
SOURCES_PDF = pdf-template-render.c mjson.c libtct.c pdfgen.c

.PHONY: all clean install example example-pdf

all: $(TARGET_DOCX) $(TARGET_PDF)

$(TARGET_DOCX): $(SOURCES_DOCX)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET_DOCX) $(SOURCES_DOCX) $(LIBS) $(LDFLAGS)

$(TARGET_PDF): $(SOURCES_PDF)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET_PDF) $(SOURCES_PDF) $(LIBS) $(LDFLAGS)

clean:
	rm -f $(TARGET_DOCX) $(TARGET_PDF) *.o
	rm -f example/output.docx example/output.pdf

install: $(TARGET_DOCX) $(TARGET_PDF)
	install -D -m 755 $(TARGET_DOCX) $(DESTDIR)/usr/local/bin/$(TARGET_DOCX)
	install -D -m 755 $(TARGET_PDF) $(DESTDIR)/usr/local/bin/$(TARGET_PDF)

uninstall:
	rm -f $(DESTDIR)/usr/local/bin/$(TARGET_DOCX)
	rm -f $(DESTDIR)/usr/local/bin/$(TARGET_PDF)

example: $(TARGET_DOCX)
	./$(TARGET_DOCX) --template-file example/sample-template.docx \
	            --json-data-file example/sample-data.json \
	            --generated-file example/output.docx
	@echo "Example output generated in example/output.docx"

example-pdf: $(TARGET_PDF)
	./$(TARGET_PDF) --template-file example/sample-template.txt \
	            --json-data-file example/sample-data.json \
	            --generated-file example/output.pdf
	@echo "Example PDF output generated in example/output.pdf"
