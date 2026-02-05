# Implementation Summary: C Versions of Template Renderers

## Overview

This implementation includes pure C programs for document template rendering:

1. **docx-template-render.c**: Generates DOCX documents from DOCX templates and JSON data
2. **pdf-template-render.c**: Generates PDF documents from plain text templates and JSON data

Both versions use single-file libraries where possible as requested (txml, libtct, mjson, and pdfgen).

## Architecture

### DOCX Renderer (docx-template-render.c)

#### Core Components

1. **docx-template-render.c** (Main Program)
   - Command-line argument parsing using getopt
   - JSON data loading and parsing
   - DOCX file extraction and manipulation
   - Template rendering integration
   - File I/O operations

2. **Dependencies** (All Included)
   - **libtct** (included): Template engine for variable substitution and control structures
   - **mjson** (included): JSON parsing library - lightweight streaming parser
   - **miniz** (included): ZIP file handling (DOCX files are ZIP archives)
   - **txml.h** (included): XML parsing header (single-file library)

#### Data Flow

```
JSON Data File → mjson Parser → tct_arguments
                                       ↓
Template DOCX → Extract document.xml → libtct Renderer → Rendered XML
                                                              ↓
                                       New DOCX ← Repackage with miniz
```

### PDF Renderer (pdf-template-render.c)

#### Core Components

1. **pdf-template-render.c** (Main Program)
   - Command-line argument parsing using getopt
   - JSON data loading and parsing
   - Plain text template reading
   - Template rendering integration
   - PDF generation
   - File I/O operations

2. **Dependencies** (All Single-File Libraries as Requested)
   - **libtct** (included): Template engine for variable substitution and control structures
   - **mjson** (included): JSON parsing library - lightweight streaming parser (single-file)
   - **pdfgen** (included): PDF generation library (single-file library from https://github.com/AndreRenaud/PDFGen)

#### Data Flow

```
JSON Data File → mjson Parser → tct_arguments
                                       ↓
Text Template → Read as string → libtct Renderer → Rendered Text
                                                       ↓
                                    PDF Document ← pdfgen formatter
```

## Key Features

### Template Syntax Support (Both Versions)

Both programs support libtct template syntax:

- **Variables**: `{{ variable_name }}`
- **Nested Objects**: `{{ object.property }}` (e.g., `{{ address.city }}`)
- **Conditionals**: `{{#if condition}} ... {{/if}}`
- **Conditionals with else**: `{{#if condition}} ... {{#else}} ... {{/if}}`
- **Loops**: `{{#each array}} ... {{/each}}`

### JSON Data Support (Both Versions)

- String values
- Numeric values (integers and floats)
- Boolean values
- Arrays (for iteration)
- Nested object structures (using dot notation)
- Up to 100 levels of nesting depth

### DOCX Handling (docx-template-render)

- Extracts `word/document.xml` from template DOCX
- Renders template with JSON data
- Repackages all DOCX components into output file
- Preserves formatting and structure

### PDF Handling (pdf-template-render)

- Reads plain text templates (easy to create and edit)
- Renders template with JSON data
- Generates PDF using pdfgen single-file library
- Automatic page breaks when content exceeds page height
- A4 page format with configurable margins
- Times-Roman font at 12pt by default

## Implementation Details

### Memory Management (Both Versions)

- All allocated memory is properly freed on cleanup
- Error paths include cleanup code
- NULL checks for all allocations

### Error Handling (Both Versions)

- Comprehensive error messages
- Graceful failure with proper exit codes
- File operation error reporting
- JSON validation with helpful error messages

### Build System

- Makefile with multiple targets (all, clean, install, example, example-pdf)
- Separate build targets for each version
- Configurable compiler flags
- Library path configuration

## Testing

### Manual Testing (Both Versions)

Successfully tested with:
- Simple variable substitution
- Nested object access (e.g., `{{ address.city }}`)
- Conditional rendering
- Array iteration
- Complex employee information template

### Test Files

Located in `example/` directory:
- `sample-template.docx`: Employee information DOCX template
- `sample-template.txt`: Employee information text template for PDF
- `sample-data.json`: Sample employee data with nested objects and arrays
- `README.md`: Usage instructions for both versions

## Comparison with Python Version

### Advantages (Both C Versions)

1. **Performance**: Significantly faster for large documents
2. **Dependencies**: No Python runtime required
3. **Deployment**: Single compiled binary per version
4. **Memory**: Lower memory footprint

### DOCX Version Specific

1. **Template Format**: Uses binary DOCX files (created in Word/LibreOffice)
2. **Output Format**: Generates DOCX files
3. **Use Case**: When you need to maintain complex formatting and styles

### PDF Version Specific

1. **Template Format**: Uses plain text files (easy to create/edit in any text editor)
2. **Output Format**: Generates PDF files directly
3. **Single-File Libraries**: Uses pdfgen as requested (single-file PDF generation)
4. **Use Case**: When you need simple, easy-to-create templates with PDF output

### Differences from Python Version

1. **Template Syntax**: Both use libtct syntax ({{ }}) instead of Jinja2
2. **JSON Support**: Full support for nested objects and arrays with dot notation
3. **Features**: Focused on core functionality with high performance

## Library Choices (Single-File Libraries as Requested)

The implementation uses single-file libraries where possible as requested:

1. **txml.h**: Single-file XML parsing library (used in DOCX version)
2. **libtct**: Template rendering library  
3. **mjson**: Single-file JSON parsing library
4. **pdfgen**: Single-file PDF generation library (pdfgen.h and pdfgen.c)
5. **miniz**: Single-file ZIP library (used in DOCX version for ZIP handling)

## Future Enhancements

Potential improvements:

1. Support for more PDF features (fonts, colors, images)
2. More complex array operations
3. Custom template functions
4. Better error messages with line numbers
5. Support for multiple document parts (headers, footers)

## Build Requirements

- GCC or compatible C compiler
- Standard C library with math support
- All dependencies are included in the repository (no external installations needed)

## CI/CD

The repository includes GitHub Actions workflows for continuous integration:
- Automated building of both DOCX and PDF versions on push and pull requests
- Example test execution for both versions
- Output verification to ensure template rendering works correctly
- Artifact uploading for both executables and generated files

## Documentation

- `BUILD.md`: Detailed build and usage instructions for both versions
- `README.md`: Overview and quick start for both versions
- `example/README.md`: Example usage guide for both versions
- `Dockerfile.c`: Docker build file for DOCX version
- `Dockerfile.pdf`: Docker build file for PDF version
- Inline code comments for maintainability

## Files Created/Modified

### New Files (PDF Version)

- `pdf-template-render.c` - PDF renderer C implementation
- `pdfgen.c`, `pdfgen.h` - PDF generation library (single-file library from PDFGen)
- `Dockerfile.pdf` - Docker build file for PDF version
- `example/sample-template.txt` - Plain text template for PDF examples

### Existing Files (DOCX Version)

- `docx-template-render.c` - DOCX renderer C implementation
- `Makefile` - Build system (updated for both versions)
- `Dockerfile.c` - Docker build file for DOCX version
- `BUILD.md` - Build documentation (updated)
- `libtct.c`, `libtct.h` - Template rendering library (from libtct)
- `mjson.c`, `mjson.h` - JSON parsing library (from mjson)
- `miniz.c`, `miniz.h` - ZIP library (single-file)
- `txml.h` - XML parsing header (single-file)
- `example/` - Example files directory
- `.github/workflows/ci.yml` - CI/CD pipeline (updated)

### Modified Files

- `README.md` - Added PDF version documentation
- `BUILD.md` - Added PDF version build instructions
- `example/README.md` - Added PDF version examples
- `example/sample-data.json` - Added nested address object
- `.gitignore` - Added PDF build artifacts
- `Makefile` - Updated to build both versions

## Conclusion

The implementation successfully adds a C PDF template renderer alongside the existing DOCX renderer. Both implementations follow C best practices for memory management and error handling.

**Key Achievements:**
1. **Single-File Libraries**: Uses txml, libtct, mjson, and pdfgen as requested, with most being single-file libraries
2. **PDF Support**: Generates PDF documents directly from plain text templates
3. **Full Template Support**: Nested objects, arrays, conditionals, and loops all work correctly
4. **Easy to Use**: Plain text templates are easier to create and edit than binary DOCX files
5. **Well-Documented**: Comprehensive documentation with examples
6. **CI/CD Tested**: Automated testing ensures reliability
7. **Docker Support**: Both versions have Docker support for easy deployment

The PDF version provides a simpler alternative when you don't need the complex formatting of DOCX files but want direct PDF output with template variables filled from JSON data.
