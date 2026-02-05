# python-docx-template-dockerfiles

[![CI](https://github.com/gerbenvoshol/python-docx-template-dockerfiles/actions/workflows/ci.yml/badge.svg)](https://github.com/gerbenvoshol/python-docx-template-dockerfiles/actions/workflows/ci.yml)

Dockerfiles for building images with LibreOffice and python-docx-template

## Overview

This repository contains Python and C implementations of document template renderers:
- **Python version**: Uses `python-docx-template` library for DOCX files
- **C DOCX version**: Pure C implementation for DOCX templates with all dependencies included
- **C PDF version**: Pure C implementation for PDF generation from text templates with all dependencies included

## Python Version

Usage example:

```bash
docker run -ti --rm --entrypoint /bin/bash -v path/to/data:/data frostasm/python-docx-template
cd /data
python3 -m docxtpl --template-file template.docx --json-data-file data.json --generated-file result.docx
soffice --convert-to pdf result.docx
```

## C Version - DOCX Renderer

The C DOCX version includes all dependencies (no external libraries required):
- libtct (template rendering, included)
- mjson (JSON parsing, included)
- miniz (ZIP/DOCX handling, included)
- txml.h (XML parsing, included)

### Building

```bash
make docx-template-render
```

### Usage

The program supports both long and short argument names:

```bash
# Long form
./docx-template-render --template-file template.docx --json-data-file data.json --generated-file result.docx

# Short form
./docx-template-render -t template.docx -j data.json -g result.docx
```

### Features

- **Nested Object Support**: Access nested JSON properties using dot notation (e.g., `{{ company.name }}`, `{{ address.city }}`)
- **Multi-Instance Safe**: Can be run with multiple instances simultaneously without conflicts
- **Short Arguments**: Supports both short (`-t`, `-j`, `-g`) and long (`--template-file`, etc.) argument names
- **Fast**: Pure C implementation for high performance

### Docker Usage

Build the Docker image:

```bash
docker build -f Dockerfile.c -t docx-template-render-c .
```

Run the container:

```bash
docker run -ti --rm --entrypoint /bin/bash -v path/to/data:/data docx-template-render-c
cd /data
# Using short argument names
docx-template-render -t template.docx -j data.json -g result.docx
soffice --convert-to pdf result.docx
```

## C Version - PDF Renderer

The C PDF version generates PDF documents from plain text templates. It includes all dependencies as single-file libraries:
- libtct (template rendering, included)
- mjson (JSON parsing, included)
- pdfgen (PDF generation, single-file library)

### Building

```bash
make pdf-template-render
```

Or build both C versions:

```bash
make all
```

### Usage

The PDF renderer takes a plain text template file (not a binary PDF) with `{{ }}` placeholders:

```bash
# Long form
./pdf-template-render --template-file template.txt --json-data-file data.json --generated-file output.pdf

# Short form
./pdf-template-render -t template.txt -j data.json -g output.pdf
```

### Example Text Template (template.txt)

```
Employee Information
====================

Name: {{ name }}
Company: {{ company }}
Position: {{ position }}
Salary: ${{ salary }}

Address:
{{ address.street }}
{{ address.city }}, {{ address.zip }}

Skills:
{{#each skills}}
- {{ skills }}
{{/each}}
```

### Features

- **Plain Text Templates**: Uses simple text files as templates (easy to create and edit)
- **PDF Generation**: Generates PDF files directly using the pdfgen single-file library
- **Nested Object Support**: Access nested JSON properties using dot notation
- **Array Support**: Iterate through arrays with `{{#each}}` blocks
- **Single-File Libraries**: Uses txml, libtct, mjson, and pdfgen as requested
- **Fast**: Pure C implementation for high performance

### Docker Usage

Build the Docker image:

```bash
docker build -f Dockerfile.pdf -t pdf-template-render .
```

Run the container:

```bash
docker run -ti --rm -v $(pwd)/data:/data pdf-template-render \
    pdf-template-render -t template.txt -j data.json -g output.pdf
```

Or run interactively:

```bash
docker run -ti --rm --entrypoint /bin/bash -v $(pwd)/data:/data pdf-template-render
cd /data
pdf-template-render -t template.txt -j data.json -g output.pdf
```

## Requirements

### C DOCX Version Dependencies

All dependencies are included in the repository:
- **libtct**: Template rendering library (from https://github.com/gerbenvoshol/libtct)
- **mjson**: JSON parsing library (from https://github.com/gerbenvoshol/mjson)
- **miniz**: ZIP file handling for DOCX files (single-file library)
- **txml.h**: XML parsing utilities (single-file library)

### C PDF Version Dependencies

All dependencies are included in the repository as single-file libraries:
- **libtct**: Template rendering library (from https://github.com/gerbenvoshol/libtct)
- **mjson**: JSON parsing library (from https://github.com/gerbenvoshol/mjson)
- **pdfgen**: PDF generation library (single-file library from https://github.com/AndreRenaud/PDFGen)

No external installation required - just run `make all` to build both C versions!