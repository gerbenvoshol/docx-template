/*
 * docx-template-render.c - Generate .docx document from docx template and json data file
 *
 * This is a C implementation of the Python script docx-template-render.py
 * using:
 * - libtct (https://github.com/gerbenvoshol/libtct) for templating
 * - miniz (from doctxt) for ZIP/DOCX file handling
 * - cJSON for JSON parsing
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

/* Include headers from libtct for template processing */
#include "libtct.h"

/* Include miniz for ZIP handling (DOCX files are ZIP archives) */
#include "miniz.h"

/* Include cJSON for JSON parsing */
#include "cJSON.h"

/* Include txml for XML parsing */
#include "txml.h"

#define PROGRAM_NAME "docx-template-render"
#define TEMP_DIR "/tmp/docx_template_render"
#define MAX_PATH 4096

/* Structure to hold DOCX document data */
typedef struct {
    mz_zip_archive zip;
    char *document_xml;
    size_t document_xml_len;
} docx_document;

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
        printf("\nDOCX templates should use {{ variable }} syntax for placeholders.\n");
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

/* Convert cJSON object to tct_arguments recursively */
static tct_arguments *
json_to_arguments(cJSON *json, tct_arguments *args)
{
    cJSON *item = NULL;
    
    if (json == NULL)
        return args;
    
    /* Handle different JSON types */
    cJSON_ArrayForEach(item, json)
    {
        if (item->string == NULL)
            continue;
            
        if (cJSON_IsString(item))
        {
            tct_add_argument(args, item->string, "%s", item->valuestring);
        }
        else if (cJSON_IsNumber(item))
        {
            if (item->valuedouble == (double)item->valueint)
            {
                tct_add_argument(args, item->string, "%d", item->valueint);
            }
            else
            {
                tct_add_argument(args, item->string, "%.2f", item->valuedouble);
            }
        }
        else if (cJSON_IsBool(item))
        {
            tct_add_argument(args, item->string, "%s", cJSON_IsTrue(item) ? "true" : "");
        }
        else if (cJSON_IsArray(item))
        {
            /* Add array items with the same name for iteration */
            cJSON *array_item = NULL;
            cJSON_ArrayForEach(array_item, item)
            {
                if (cJSON_IsString(array_item))
                {
                    tct_add_argument(args, item->string, "%s", array_item->valuestring);
                }
                else if (cJSON_IsNumber(array_item))
                {
                    if (array_item->valuedouble == (double)array_item->valueint)
                    {
                        tct_add_argument(args, item->string, "%d", array_item->valueint);
                    }
                    else
                    {
                        tct_add_argument(args, item->string, "%.2f", array_item->valuedouble);
                    }
                }
            }
        }
        /* Note: Nested objects are not fully supported in this simple implementation */
    }
    
    return args;
}

/* Extract document.xml from DOCX file */
static char *
extract_document_xml(const char *docx_path, size_t *out_len)
{
    mz_zip_archive zip;
    char *xml_content = NULL;
    size_t xml_size;
    
    memset(&zip, 0, sizeof(zip));
    
    if (!mz_zip_reader_init_file(&zip, docx_path, 0))
    {
        fprintf(stderr, "Error: Failed to open DOCX file as ZIP archive\n");
        return NULL;
    }
    
    /* Extract word/document.xml */
    xml_content = (char *)mz_zip_reader_extract_file_to_heap(&zip, "word/document.xml", 
                                                               &xml_size, 0);
    if (xml_content == NULL)
    {
        fprintf(stderr, "Error: Failed to extract document.xml from DOCX\n");
        mz_zip_reader_end(&zip);
        return NULL;
    }
    
    mz_zip_reader_end(&zip);
    
    if (out_len != NULL)
        *out_len = xml_size;
    
    return xml_content;
}

/* Create a new DOCX file by copying template and replacing document.xml */
static int
create_docx_with_content(const char *template_path, const char *output_path, 
                         const char *new_document_xml, size_t xml_len)
{
    mz_zip_archive src_zip, dst_zip;
    int i, n, ret = -1;
    
    memset(&src_zip, 0, sizeof(src_zip));
    memset(&dst_zip, 0, sizeof(dst_zip));
    
    /* Open source DOCX */
    if (!mz_zip_reader_init_file(&src_zip, template_path, 0))
    {
        fprintf(stderr, "Error: Failed to open template DOCX file\n");
        return -1;
    }
    
    /* Create destination DOCX */
    if (!mz_zip_writer_init_file(&dst_zip, output_path, 0))
    {
        fprintf(stderr, "Error: Failed to create output DOCX file\n");
        mz_zip_reader_end(&src_zip);
        return -1;
    }
    
    /* Copy all files from source except document.xml */
    n = mz_zip_reader_get_num_files(&src_zip);
    for (i = 0; i < n; i++)
    {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&src_zip, i, &file_stat))
            continue;
        
        /* Skip document.xml - we'll add it separately */
        if (strcmp(file_stat.m_filename, "word/document.xml") == 0)
            continue;
        
        /* Copy file from source to destination */
        if (!mz_zip_writer_add_from_zip_reader(&dst_zip, &src_zip, i))
        {
            fprintf(stderr, "Error: Failed to copy %s\n", file_stat.m_filename);
            goto cleanup;
        }
    }
    
    /* Add the new document.xml */
    if (!mz_zip_writer_add_mem(&dst_zip, "word/document.xml", 
                                new_document_xml, xml_len, MZ_DEFAULT_COMPRESSION))
    {
        fprintf(stderr, "Error: Failed to add document.xml to output\n");
        goto cleanup;
    }
    
    ret = 0;
    
cleanup:
    mz_zip_writer_finalize_archive(&dst_zip);
    mz_zip_writer_end(&dst_zip);
    mz_zip_reader_end(&src_zip);
    
    return ret;
}

int
main(int argc, char **argv)
{
    const char *template_file = NULL;
    const char *json_data_file = NULL;
    const char *generated_file = NULL;
    char *json_data = NULL;
    char *document_xml = NULL;
    char *rendered_xml = NULL;
    size_t xml_len;
    cJSON *json_root = NULL;
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

    /* Parse JSON data */
    json_root = cJSON_Parse(json_data);
    if (json_root == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "Error: Failed to parse JSON before: %s\n", error_ptr);
        }
        else
        {
            fprintf(stderr, "Error: Failed to parse JSON data\n");
        }
        goto cleanup;
    }

    /* Convert JSON to template arguments */
    args = json_to_arguments(json_root, args);
    if (args == NULL)
    {
        fprintf(stderr, "Warning: No arguments extracted from JSON\n");
        /* Continue anyway - empty arguments is valid */
    }

    /* Extract document.xml from template DOCX */
    document_xml = extract_document_xml(template_file, &xml_len);
    if (document_xml == NULL)
    {
        fprintf(stderr, "Error: Failed to extract document.xml from template\n");
        goto cleanup;
    }

    /* Null-terminate the XML string */
    char *xml_str = (char *)malloc(xml_len + 1);
    if (xml_str == NULL)
    {
        fprintf(stderr, "Error: Failed to allocate memory for XML\n");
        goto cleanup;
    }
    memcpy(xml_str, document_xml, xml_len);
    xml_str[xml_len] = '\0';
    free(document_xml);
    document_xml = xml_str;

    /* Render template with arguments */
    rendered_xml = tct_render(document_xml, args);
    if (rendered_xml == NULL)
    {
        fprintf(stderr, "Error: Failed to render template\n");
        goto cleanup;
    }

    /* Create output DOCX with rendered content */
    if (create_docx_with_content(template_file, generated_file, 
                                 rendered_xml, strlen(rendered_xml)) != 0)
    {
        fprintf(stderr, "Error: Failed to create output DOCX file\n");
        goto cleanup;
    }

    /* Success */
    printf("Successfully generated '%s' from template '%s' with data '%s'\n",
           generated_file, template_file, json_data_file);
    ret = EXIT_SUCCESS;

cleanup:
    if (json_data != NULL)
        free(json_data);
    if (json_root != NULL)
        cJSON_Delete(json_root);
    if (args != NULL)
        tct_free_argument(args);
    if (document_xml != NULL)
        free(document_xml);
    if (rendered_xml != NULL)
        free(rendered_xml);

    return ret;
}
