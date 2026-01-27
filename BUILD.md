# Building and Testing docx-template-render (C version)

## Prerequisites

The C program requires:
- GCC compiler (or compatible C compiler)
- libtct library (https://github.com/gerbenvoshol/libtct)
- Standard C library with math support

The following dependencies are included in this repository:
- cJSON (for JSON parsing)
- miniz (for ZIP/DOCX file handling)
- txml.h (for XML parsing, from doctxt repository)

## Building

### Install libtct

```bash
cd /tmp
git clone https://github.com/gerbenvoshol/libtct.git
cd libtct
make
sudo make install
```

### Build docx-template-render

```bash
make clean
make
```

This will create the `docx-template-render` executable.

## Testing

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

```bash
./docx-template-render \
    --template-file template.docx \
    --json-data-file data.json \
    --generated-file output.docx
```

### 4. Verify the output

Open `output.docx` in Microsoft Word or LibreOffice, or extract and view the content:

```bash
unzip -p output.docx word/document.xml
```

## Template Syntax

The C program uses libtct template syntax which is compatible with {{ variable }} style placeholders:

- **Variable substitution**: `{{ variable_name }}`
- **Conditional blocks**: `{{#if condition}} ... {{/if}}`
- **Conditional with else**: `{{#if condition}} ... {{#else}} ... {{/if}}`
- **Loop blocks**: `{{#each items}} ... {{/each}}`

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

## Differences from Python version

The C version has the following differences compared to the Python version:

1. **JSON Parsing**: Limited to flat objects and simple arrays. Deeply nested objects may not be fully supported.
2. **Template Syntax**: Uses libtct syntax ({{ }}) instead of Jinja2 syntax used by python-docx-template.
3. **Performance**: The C version is significantly faster for large documents.
4. **Dependencies**: Requires fewer runtime dependencies (no Python interpreter needed).

## Troubleshooting

### Build Errors

If you get "libtct not found" errors:
- Ensure libtct is installed in `/usr/local/lib` and `/usr/local/include`
- Run `sudo ldconfig` after installing libtct

### Runtime Errors

If the program fails to open DOCX files:
- Ensure the input file is a valid DOCX (ZIP) file
- Check file permissions
- Verify the DOCX file contains a `word/document.xml` entry
