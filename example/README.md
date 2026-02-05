# Example Usage

This directory contains example files for testing both the docx-template-render and pdf-template-render C programs.

## Files

- `sample-data.json` - Sample JSON data file with employee information (works for both DOCX and PDF versions)
- `sample-template.docx` - Sample DOCX template with placeholders
- `sample-template.txt` - Sample plain text template for PDF generation

## DOCX Template Example

### Creating a Test Template

To create a test template:

1. Open Microsoft Word or LibreOffice Writer
2. Create a new document with the following content:

```
EMPLOYEE INFORMATION

Name: {{ name }}
Position: {{ position }}
Department: {{ department }}
Manager: {{ manager }}
Start Date: {{ start_date }}
Salary: ${{ salary }}

Address:
{{ address.street }}
{{ address.city }}, {{ address.zip }}

Contact:
Email: {{ email }}
Phone: {{ phone }}

{{#if remote}}
Work Mode: Remote
{{/if}}

Skills:
{{#each skills}}
- {{ skills }}
{{/each}}

Company: {{ company }}
```

3. Save the file as `sample-template.docx` in this directory

### Running the DOCX Example

From the repository root:

```bash
./docx-template-render \
    --template-file example/sample-template.docx \
    --json-data-file example/sample-data.json \
    --generated-file example/output.docx
```

Or using make:

```bash
make example
```

### Verifying the DOCX Output

View the generated content:

```bash
unzip -p example/output.docx word/document.xml | xmllint --format -
```

Or open `example/output.docx` in Microsoft Word or LibreOffice Writer.

## PDF Template Example

### Using the Plain Text Template

The `sample-template.txt` file is a plain text template that works with the PDF renderer. You can view or edit it with any text editor.

### Running the PDF Example

From the repository root:

```bash
./pdf-template-render \
    --template-file example/sample-template.txt \
    --json-data-file example/sample-data.json \
    --generated-file example/output.pdf
```

Or using make:

```bash
make example-pdf
```

### Verifying the PDF Output

Open `example/output.pdf` in any PDF viewer, or check the file:

```bash
file example/output.pdf
```

## Template Syntax

Both versions use the same libtct template syntax:

- **Variables**: `{{ variable_name }}`
- **Nested Objects**: `{{ object.property }}` (e.g., `{{ address.city }}`)
- **Conditionals**: `{{#if condition}} ... {{/if}}`
- **Conditionals with else**: `{{#if condition}} ... {{#else}} ... {{/if}}`
- **Loops**: `{{#each array}} ... {{/each}}`

The main difference is that:
- **DOCX version**: Uses binary DOCX files as templates (created in Word/LibreOffice)
- **PDF version**: Uses plain text files as templates (easy to create/edit in any text editor)
