/*
 * pdf-template-render.c - Generate PDF document from text template and json data file
 *
 * This is a C implementation that generates PDFs from text templates
 * using:
 * - libtct (https://github.com/gerbenvoshol/libtct) for templating
 * - pdfgen.h for PDF generation (single-file library)
 * - mjson for JSON parsing
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <ctype.h>

/* Include headers from libtct for template processing */
#include "libtct.h"

/* Include mjson for JSON parsing */
#include "mjson.h"

/* Include pdfgen for PDF generation */
#include "pdfgen.h"

#define PROGRAM_NAME "pdf-template-render"
#define MAX_PATH 4096
#define MAX_NESTING_DEPTH 100
#define DEFAULT_FONT_SIZE 12
#define LINE_HEIGHT 15
#define MARGIN_LEFT 50
#define MARGIN_TOP 50
#define MARGIN_RIGHT 50
#define PAGE_WIDTH PDF_A4_WIDTH
#define PAGE_HEIGHT PDF_A4_HEIGHT

static void
usage(int status)
{
    if (status != EXIT_SUCCESS)
        fprintf(stderr, "Try '%s --help' for more information.\n", PROGRAM_NAME);
    else
    {
        printf("Usage: %s [OPTIONS]\n", PROGRAM_NAME);
        printf("Generate PDF document from text template and json data file\n\n");
        printf("Options:\n");
        printf("  -t, --template-file FILE     Input text template file\n");
        printf("  -j, --json-data-file FILE    Input JSON data file\n");
        printf("  -g, --generated-file FILE    Output PDF file\n");
        printf("  -h, --help                   Display this help and exit\n");
        printf("\nText templates should use {{ variable }} syntax for placeholders.\n");
        printf("The template is a plain text file that will be rendered as PDF.\n");
    }
    exit(status);
}

static char *
read_file_contents(const char *filename)
{
    FILE *fp;
    char *content;
    long file_size;
    size_t read_size;

    fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: Cannot open file '%s': %s\n", filename, strerror(errno));
        return NULL;
    }

    /* Get file size */
    if (fseek(fp, 0, SEEK_END) != 0)
    {
        fprintf(stderr, "Error: Cannot seek file '%s': %s\n", filename, strerror(errno));
        fclose(fp);
        return NULL;
    }

    file_size = ftell(fp);
    if (file_size < 0)
    {
        fprintf(stderr, "Error: Cannot get file size '%s': %s\n", filename, strerror(errno));
        fclose(fp);
        return NULL;
    }

    rewind(fp);

    /* Allocate memory for file content */
    content = (char *)malloc(file_size + 1);
    if (content == NULL)
    {
        fprintf(stderr, "Error: Cannot allocate memory for file '%s'\n", filename);
        fclose(fp);
        return NULL;
    }

    /* Read file content */
    read_size = fread(content, 1, file_size, fp);
    if (read_size != (size_t)file_size)
    {
        fprintf(stderr, "Error: Cannot read file '%s'\n", filename);
        free(content);
        fclose(fp);
        return NULL;
    }

    content[file_size] = '\0';
    fclose(fp);

    return content;
}

/* Forward declaration for recursive call */
static void process_json_object(tct_arguments **args, const char *json, int json_len, 
                                 const char *prefix, int depth);

/* Process a JSON value and add it to arguments with the given key */
static void
process_json_value(tct_arguments **args, const char *key, 
                   const char *value_start, int vlen, int vtype, int depth)
{
    if (vtype == MJSON_TOK_STRING)
    {
        /* String value - extract without quotes */
        if (vlen > 2 && value_start[0] == '"')
        {
            char value[1024];
            int copy_len = vlen - 2;
            if (copy_len >= (int)sizeof(value))
                copy_len = sizeof(value) - 1;
            memcpy(value, value_start + 1, copy_len);
            value[copy_len] = '\0';
            tct_add_argument(*args, (char *)key, "%s", value);
        }
    }
    else if (vtype == MJSON_TOK_NUMBER)
    {
        /* Number value */
        char num_str[64];
        if (vlen < (int)sizeof(num_str))
        {
            memcpy(num_str, value_start, vlen);
            num_str[vlen] = '\0';
            double num = atof(num_str);
            
            /* Check if it's an integer */
            if (num == (long)num)
            {
                tct_add_argument(*args, (char *)key, "%ld", (long)num);
            }
            else
            {
                tct_add_argument(*args, (char *)key, "%.2f", num);
            }
        }
    }
    else if (vtype == MJSON_TOK_TRUE)
    {
        /* Boolean true value */
        tct_add_argument(*args, (char *)key, "%s", "true");
    }
    else if (vtype == MJSON_TOK_FALSE)
    {
        /* Boolean false value - use empty string */
        tct_add_argument(*args, (char *)key, "%s", "");
    }
    else if (vtype == MJSON_TOK_ARRAY)
    {
        /* Array value - iterate through array elements */
        int aoff, akoff, aklen, avoff, avlen, avtype;
        
        /* Iterate through array elements */
        for (aoff = 0; (aoff = mjson_next(value_start, vlen, aoff, 
                                           &akoff, &aklen, &avoff, &avlen, &avtype)) != 0; )
        {
            if (avtype == MJSON_TOK_STRING)
            {
                /* Extract string from array (without quotes) */
                if (avlen > 2 && value_start[avoff] == '"')
                {
                    char str_val[1024];
                    int copy_len = avlen - 2;
                    if (copy_len >= (int)sizeof(str_val))
                        copy_len = sizeof(str_val) - 1;
                    memcpy(str_val, value_start + avoff + 1, copy_len);
                    str_val[copy_len] = '\0';
                    tct_add_argument(*args, (char *)key, "%s", str_val);
                }
            }
            else if (avtype == MJSON_TOK_NUMBER)
            {
                /* Extract number from array */
                char num_str[64];
                if (avlen < (int)sizeof(num_str))
                {
                    memcpy(num_str, value_start + avoff, avlen);
                    num_str[avlen] = '\0';
                    double num = atof(num_str);
                    
                    if (num == (long)num)
                    {
                        tct_add_argument(*args, (char *)key, "%ld", (long)num);
                    }
                    else
                    {
                        tct_add_argument(*args, (char *)key, "%.2f", num);
                    }
                }
            }
        }
    }
    else if (vtype == MJSON_TOK_OBJECT)
    {
        /* Nested object - recursively process with dotted prefix */
        /* Check recursion depth to prevent stack overflow */
        if (depth >= MAX_NESTING_DEPTH)
        {
            fprintf(stderr, "Warning: Maximum nesting depth (%d) reached for key '%s', skipping nested object\n", 
                    MAX_NESTING_DEPTH, key);
            return;
        }
        process_json_object(args, value_start, vlen, key, depth + 1);
    }
}

/* Recursively process JSON object and add all properties to arguments */
static void
process_json_object(tct_arguments **args, const char *json, int json_len, 
                    const char *prefix, int depth)
{
    int koff, klen, voff, vlen, vtype, off;
    
    /* Iterate through JSON object properties using mjson_next */
    for (off = 0; (off = mjson_next(json, json_len, off, &koff, &klen, &voff, &vlen, &vtype)) != 0; )
    {
        /* Extract key name */
        char key_buf[256];
        char full_key[512];
        
        if (klen >= (int)sizeof(key_buf))
            continue;
        
        /* Get the key string (remove quotes if present) */
        if (json[koff] == '"' && klen >= 2)
        {
            memcpy(key_buf, json + koff + 1, klen - 2);
            key_buf[klen - 2] = '\0';
        }
        else
        {
            memcpy(key_buf, json + koff, klen);
            key_buf[klen] = '\0';
        }
        
        /* Build full key with prefix if present */
        if (prefix != NULL && prefix[0] != '\0')
        {
            int ret = snprintf(full_key, sizeof(full_key), "%s.%s", prefix, key_buf);
            /* Check if the key was truncated */
            if (ret >= (int)sizeof(full_key))
            {
                fprintf(stderr, "Warning: Key path too long (truncated): %s.%s\n", prefix, key_buf);
                continue;
            }
        }
        else
        {
            snprintf(full_key, sizeof(full_key), "%s", key_buf);
        }
        
        /* Process the value recursively */
        process_json_value(args, full_key, json + voff, vlen, vtype, depth);
    }
}

/* Convert JSON object to tct_arguments using mjson */
static tct_arguments *
json_to_arguments(const char *json, int json_len)
{
    tct_arguments *args = NULL;
    
    if (json == NULL || json_len <= 0)
        return NULL;
    
    /* Process the root JSON object */
    process_json_object(&args, json, json_len, NULL, 0);
    
    return args;
}

/* Generate PDF from rendered text */
static int
generate_pdf(const char *text, const char *output_path)
{
    struct pdf_info info = {
        .creator = "pdf-template-render",
        .producer = "pdf-template-render",
        .title = "Generated Document",
        .author = "",
        .subject = "",
        .date = ""
    };
    
    struct pdf_doc *pdf = pdf_create(PAGE_WIDTH, PAGE_HEIGHT, &info);
    if (pdf == NULL)
    {
        fprintf(stderr, "Error: Failed to create PDF document\n");
        return -1;
    }
    
    pdf_set_font(pdf, "Times-Roman");
    pdf_append_page(pdf);
    
    /* Process text line by line */
    const char *line_start = text;
    const char *p = text;
    float y_pos = PAGE_HEIGHT - MARGIN_TOP;
    char line[1024];
    
    while (*p != '\0')
    {
        if (*p == '\n' || *(p + 1) == '\0')
        {
            /* Extract line */
            size_t line_len = p - line_start;
            if (*(p + 1) == '\0' && *p != '\n')
                line_len = p - line_start + 1;
            
            if (line_len >= sizeof(line))
                line_len = sizeof(line) - 1;
            
            memcpy(line, line_start, line_len);
            line[line_len] = '\0';
            
            /* Check if we need a new page */
            if (y_pos < MARGIN_TOP)
            {
                pdf_append_page(pdf);
                y_pos = PAGE_HEIGHT - MARGIN_TOP;
            }
            
            /* Add text to PDF */
            pdf_add_text(pdf, NULL, line, DEFAULT_FONT_SIZE, MARGIN_LEFT, y_pos, PDF_BLACK);
            y_pos -= LINE_HEIGHT;
            
            line_start = p + 1;
        }
        p++;
    }
    
    /* Save PDF */
    if (pdf_save(pdf, output_path) < 0)
    {
        fprintf(stderr, "Error: Failed to save PDF file '%s'\n", output_path);
        pdf_destroy(pdf);
        return -1;
    }
    
    pdf_destroy(pdf);
    return 0;
}

int
main(int argc, char **argv)
{
    const char *template_file = NULL;
    const char *json_data_file = NULL;
    const char *generated_file = NULL;
    char *json_data = NULL;
    char *template_text = NULL;
    char *rendered_text = NULL;
    tct_arguments *args = NULL;
    int c;
    int ret = EXIT_FAILURE;

    /* Parse command line arguments */
    static struct option long_options[] = {
        {"template-file", required_argument, 0, 't'},
        {"json-data-file", required_argument, 0, 'j'},
        {"generated-file", required_argument, 0, 'g'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    while (1)
    {
        int option_index = 0;
        c = getopt_long(argc, argv, "t:j:g:h", long_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
        case 't':
            template_file = optarg;
            break;
        case 'j':
            json_data_file = optarg;
            break;
        case 'g':
            generated_file = optarg;
            break;
        case 'h':
            usage(EXIT_SUCCESS);
            break;
        case '?':
            usage(EXIT_FAILURE);
            break;
        default:
            abort();
        }
    }

    /* Validate required arguments */
    if (template_file == NULL || json_data_file == NULL || generated_file == NULL)
    {
        fprintf(stderr, "Error: All three arguments are required: --template-file, --json-data-file, and --generated-file\n");
        usage(EXIT_FAILURE);
    }

    /* Read JSON data file */
    json_data = read_file_contents(json_data_file);
    if (json_data == NULL)
    {
        fprintf(stderr, "Error: Failed to read JSON data file\n");
        goto cleanup;
    }

    /* Validate JSON data with mjson */
    int json_len = strlen(json_data);
    if (mjson(json_data, json_len, NULL, NULL) < 0)
    {
        fprintf(stderr, "Error: Failed to parse JSON data - invalid JSON format\n");
        goto cleanup;
    }

    /* Convert JSON to template arguments */
    args = json_to_arguments(json_data, json_len);
    /* Note: args can be NULL for empty JSON, which is valid */

    /* Read template text file */
    template_text = read_file_contents(template_file);
    if (template_text == NULL)
    {
        fprintf(stderr, "Error: Failed to read template text file\n");
        goto cleanup;
    }

    /* Render template with arguments */
    rendered_text = tct_render(template_text, args);
    if (rendered_text == NULL)
    {
        fprintf(stderr, "Error: Failed to render template\n");
        goto cleanup;
    }

    /* Generate PDF with rendered content */
    if (generate_pdf(rendered_text, generated_file) != 0)
    {
        fprintf(stderr, "Error: Failed to generate PDF file\n");
        goto cleanup;
    }

    /* Success */
    printf("Successfully generated '%s' from template '%s' with data '%s'\n",
           generated_file, template_file, json_data_file);
    ret = EXIT_SUCCESS;

cleanup:
    if (json_data != NULL)
        free(json_data);
    if (args != NULL)
        tct_free_argument(args);
    if (template_text != NULL)
        free(template_text);
    if (rendered_text != NULL)
        free(rendered_text);

    return ret;
}
