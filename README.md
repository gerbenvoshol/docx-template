# python-docx-template-dockerfiles

[![CI](https://github.com/gerbenvoshol/python-docx-template-dockerfiles/actions/workflows/ci.yml/badge.svg)](https://github.com/gerbenvoshol/python-docx-template-dockerfiles/actions/workflows/ci.yml)

Dockerfiles for building images with LibreOffice and python-docx-template

## Overview

This repository contains both Python and C implementations of a DOCX template renderer:
- **Python version**: Uses `python-docx-template` library
- **C version**: Pure C implementation with all dependencies included

## Python Version

Usage example:

```bash
docker run -ti --rm --entrypoint /bin/bash -v path/to/data:/data frostasm/python-docx-template
cd /data
python3 -m docxtpl --template-file template.docx --json-data-file data.json --generated-file result.docx
soffice --convert-to pdf result.docx
```

## C Version

The C version includes all dependencies (no external libraries required):
- libtct (template rendering, included)
- mjson (JSON parsing, included)
- miniz (ZIP/DOCX handling, included)
- txml.h (XML parsing, included)

### Building

```bash
make
```

### Usage

```bash
./docx-template-render --template-file template.docx --json-data-file data.json --generated-file result.docx
```

### Docker Usage

Build the Docker image:

```bash
docker build -f Dockerfile.c -t docx-template-render-c .
```

Run the container:

```bash
docker run -ti --rm --entrypoint /bin/bash -v path/to/data:/data docx-template-render-c
cd /data
docx-template-render --template-file template.docx --json-data-file data.json --generated-file result.docx
soffice --convert-to pdf result.docx
```

## Requirements

### C Version Dependencies

All dependencies are included in the repository:
- **libtct**: Template rendering library (from https://github.com/gerbenvoshol/libtct)
- **mjson**: JSON parsing library (from https://github.com/gerbenvoshol/mjson)
- **miniz**: ZIP file handling for DOCX files
- **txml.h**: XML parsing utilities

No external installation required - just run `make` to build!