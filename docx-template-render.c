/*
 * docx-template-render.c - Generate .docx document from docx template and json data file
 *
 * This is a C implementation of the Python script docx-template-render.py
 * using libtct (https://github.com/gerbenvoshol/libtct) for templating
 * and doctxt (https://github.com/gerbenvoshol/doctxt) for DOCX handling.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

/* Include headers from libtct and doctxt */
#include "tct.h"
#include "doctxt.h"

#define PROGRAM_NAME "docx-template-render"

static void
usage(int status)
{
    if (status != EXIT_SUCCESS)
        fprintf(stderr, "Try '%s --help' for more information.\n", PROGRAM_NAME);
    else
    {
        printf("Usage: %s --template-file FILE --json-data-file FILE --generated-file FILE\n", PROGRAM_NAME);
        printf("Generate .docx document from docx template and json data file\n\n");
        printf("Options:\n");
        printf("  --template-file FILE     Input DOCX template file\n");
        printf("  --json-data-file FILE    Input JSON data file\n");
        printf("  --generated-file FILE    Output DOCX file\n");
        printf("  --help                   Display this help and exit\n");
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

int
main(int argc, char **argv)
{
    const char *template_file = NULL;
    const char *json_data_file = NULL;
    const char *generated_file = NULL;
    char *json_data = NULL;
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

    /* Load DOCX template */
    doctxt_t *doc = doctxt_open(template_file);
    if (doc == NULL)
    {
        fprintf(stderr, "Error: Failed to open DOCX template file '%s'\n", template_file);
        goto cleanup;
    }

    /* Parse JSON data */
    tct_value_t *data = tct_parse_json(json_data);
    if (data == NULL)
    {
        fprintf(stderr, "Error: Failed to parse JSON data\n");
        doctxt_close(doc);
        goto cleanup;
    }

    /* Render template with JSON data */
    if (doctxt_render_template(doc, data) != 0)
    {
        fprintf(stderr, "Error: Failed to render template\n");
        tct_value_free(data);
        doctxt_close(doc);
        goto cleanup;
    }

    /* Save the generated document */
    if (doctxt_save(doc, generated_file) != 0)
    {
        fprintf(stderr, "Error: Failed to save generated file '%s'\n", generated_file);
        tct_value_free(data);
        doctxt_close(doc);
        goto cleanup;
    }

    /* Success */
    printf("Successfully generated '%s' from template '%s' with data '%s'\n",
           generated_file, template_file, json_data_file);
    ret = EXIT_SUCCESS;

    /* Cleanup */
    tct_value_free(data);
    doctxt_close(doc);

cleanup:
    if (json_data != NULL)
        free(json_data);

    return ret;
}
