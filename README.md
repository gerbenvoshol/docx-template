# python-docx-template-dockerfiles
Dockerfiles for building images with LibreOffice and python-docx-template

## Overview

This repository contains both Python and C implementations of a DOCX template renderer:
- **Python version**: Uses `python-docx-template` library
- **C version**: Pure C implementation using [libtct](https://github.com/gerbenvoshol/libtct) and [doctxt](https://github.com/gerbenvoshol/doctxt)

## Python Version

Usage example:

```bash
docker run -ti --rm --entrypoint /bin/bash -v path/to/data:/data frostasm/python-docx-template
cd /data
python3 -m docxtpl --template-file template.docx --json-data-file data.json --generated-file result.docx
soffice --convert-to pdf result.docx
```

## C Version

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
- libtct: Template rendering library
- doctxt: DOCX file manipulation library
- libzip: ZIP file handling (required by doctxt)
- libxml2: XML parsing (required by doctxt)