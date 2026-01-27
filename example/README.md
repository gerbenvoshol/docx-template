# Example Usage

This directory contains example files for testing the docx-template-render C program.

## Files

- `sample-data.json` - Sample JSON data file with employee information
- `sample-template.docx` - Sample DOCX template with placeholders (to be created)

## Creating a Test Template

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

## Running the Example

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

## Verifying the Output

View the generated content:

```bash
unzip -p example/output.docx word/document.xml | xmllint --format -
```

Or open `example/output.docx` in Microsoft Word or LibreOffice Writer.
