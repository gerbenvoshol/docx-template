# Building and Testing C Template Renderers

## Prerequisites

The C programs require:
- GCC compiler (or compatible C compiler)
- Standard C library with math support

The following dependencies are included in this repository as single-file libraries where possible:
- libtct (for template rendering)
- mjson (for JSON parsing)
- miniz (for ZIP/DOCX file handling)
- txml.h (for XML parsing, single-file library)
- pdfgen (for PDF generation, single-file library)

## Building

### Build Both C Versions

```bash
make clean
make all
```

This will create both `docx-template-render` and `pdf-template-render` executables.

### Build Only DOCX Version

```bash
make docx-template-render
```

### Build Only PDF Version

```bash
make pdf-template-render
```

## Testing - DOCX Version

A simple test can be performed as follows:

### 1. Create a test JSON file (data.json):

```json
{
    "name": "John Doe",
    "company": "Acme Corp",
    "position": "Software Engineer",
    "salary": 75000
}
```

### 2. Create a DOCX template with placeholders

Create a DOCX file with placeholders using {{ variable }} syntax in the document text.
For example, your template might contain:

```
Name: {{ name }}
Company: {{ company }}
Position: {{ position }}
Salary: {{ salary }}
```

### 3. Run the program

The program supports both long and short argument names:

```bash
# Using long argument names
./docx-template-render \
    --template-file template.docx \
    --json-data-file data.json \
    --generated-file output.docx

# Using short argument names
./docx-template-render -t template.docx -j data.json -g output.docx
```

### 4. Verify the output

Open `output.docx` in Microsoft Word or LibreOffice, or extract and view the content:

```bash
unzip -p output.docx word/document.xml
```

## Template Syntax

The C program uses libtct template syntax which is compatible with {{ variable }} style placeholders:

- **Variable substitution**: `{{ variable_name }}`
- **Nested object access**: `{{ object.property }}` or `{{ object.nested.property }}`
- **Conditional blocks**: `{{#if condition}} ... {{/if}}`
- **Conditional with else**: `{{#if condition}} ... {{#else}} ... {{/if}}`
- **Loop blocks**: `{{#each items}} ... {{/each}}`

### Nested Objects in JSON

The program fully supports nested objects using dot notation:

```json
{
    "name": "John Doe",
    "company": {
        "name": "Acme Corporation",
        "address": {
            "street": "123 Main St",
            "city": "Springfield",
            "zip": "12345"
        }
    }
}
```

In your template, access nested properties with dots:

```
Name: {{ name }}
Company: {{ company.name }}
City: {{ company.address.city }}
```

### Arrays in JSON

To use loops, provide arrays in your JSON data:

```json
{
    "items": ["Apple", "Banana", "Cherry"]
}
```

Then in your template:

```
{{#each items}}
- {{ items }}
{{/each}}
```

## Testing - PDF Version

The PDF version generates PDF documents from plain text templates.

### 1. Create a test JSON file (data.json):

Use the same JSON format as the DOCX version:

```json
{
    "name": "John Doe",
    "company": "Acme Corp",
    "position": "Software Engineer",
    "salary": 75000,
    "address": {
        "street": "123 Main St",
        "city": "Springfield",
        "zip": "12345"
    },
    "skills": ["C", "Python", "JavaScript"]
}
```

### 2. Create a plain text template (template.txt)

Create a text file with placeholders using {{ variable }} syntax:

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

### 3. Run the PDF generator

The program supports both long and short argument names:

```bash
# Using long argument names
./pdf-template-render \
    --template-file template.txt \
    --json-data-file data.json \
    --generated-file output.pdf

# Using short argument names
./pdf-template-render -t template.txt -j data.json -g output.pdf

# Or use the Makefile
make example-pdf
```

### 4. Verify the output

Open `output.pdf` in any PDF viewer, or check the file type:

```bash
file output.pdf
```

## Differences from Python version

The C DOCX version has the following differences compared to the Python version:

1. **JSON Parsing**: Supports nested objects with dot notation (e.g., `{{ company.name }}`, `{{ address.city }}`). Arrays and complex data structures are fully supported.
2. **Template Syntax**: Uses libtct syntax ({{ }}) instead of Jinja2 syntax used by python-docx-template.
3. **Performance**: The C version is significantly faster for large documents.
4. **Dependencies**: Requires fewer runtime dependencies (no Python interpreter needed).
5. **Multiple Instances**: The program can be run with multiple separate process instances simultaneously without conflicts (no shared global state).

The C PDF version has these characteristics:

1. **Plain Text Templates**: Uses simple text files instead of binary DOCX templates (easier to create and edit).
2. **PDF Output**: Generates PDF files directly without needing LibreOffice conversion.
3. **Single-File Libraries**: Uses pdfgen as a single-file library for PDF generation as requested.
4. **Template Syntax**: Same libtct syntax ({{ }}) with support for nested objects and arrays.
5. **Performance**: Fast C implementation with minimal dependencies.

## Troubleshooting

### Build Errors

If you get compilation errors:
- Ensure you have GCC or a compatible C compiler installed
- Check that all source files are present in the directory

### Runtime Errors

If the program fails to open DOCX files:
- Ensure the input file is a valid DOCX (ZIP) file
- Check file permissions
- Verify the DOCX file contains a `word/document.xml` entry
