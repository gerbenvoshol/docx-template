#include "libtct.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <stdarg.h> 

tct_arguments* tct_add_argument_(tct_arguments *next_argument, char *name, const char *format, ...) {
    va_list argp;
    tct_arguments *argument;
    size_t arg_len, name_len;

    name_len = strlen(name);
    va_start(argp, format);
    arg_len = vsnprintf(NULL, 0, format, argp);
    va_end(argp);
    argument = calloc(1, sizeof(tct_arguments) + name_len+ 1 + arg_len + 1);
    strcpy(argument->data, name);
    va_start(argp, format);
    vsnprintf(&argument->data[name_len + 1], arg_len + 1, format, argp);
    va_end(argp);
    argument->next = next_argument;

    return argument;
}

tct_arguments* tct_find_arguments(tct_arguments *arguments, char *name, size_t name_len) {
    while(arguments)
        if(memcmp(arguments->data, name, name_len) == 0 && arguments->data[name_len] == 0)
            return arguments;
        else
            arguments = arguments->next;
    return NULL;
}

char* tct_get_valuen(tct_arguments *arguments, char *name, size_t name_len) {
    tct_arguments *argument = tct_find_arguments(arguments, name, name_len);
    return argument ? &argument->data[name_len + 1] : "";
}

/* Helper function to find the next argument with the same name (for array iteration) */
static tct_arguments* tct_find_next_argument(tct_arguments *current, char *name, size_t name_len) {
    if (!current || !current->next) return NULL;
    return tct_find_arguments(current->next, name, name_len);
}

static bool tct_find_symbol(char *template, char** start_, char** end_) {
    char* start;
    char* end;

    *start_ = NULL;
    *end_ = NULL;

    start = template;
    while(*start) {
        if(memcmp(start, TCT_START_SIGN, TCT_START_SIGN_LEN) != 0) {
            start++; continue;
        }
        end = start + TCT_START_SIGN_LEN;
        while(*end) {
            if(memcmp(end, TCT_END_SIGN, TCT_END_SIGN_LEN) != 0) {
                end++; continue;
            }
            *start_ = start;
            *end_ = end;
            return true;
        }
        break;
    }
    return false;
}

void tct_free_argument(tct_arguments* arguments) {
    while (arguments) {
        tct_arguments *argument = arguments;
        arguments = argument->next;
        free(argument);
    }
}

/* Helper function to check if a string represents a "truthy" value */
static bool tct_is_truthy(const char *value) {
    if (!value || value[0] == '\0') return false;
    if (strcmp(value, "0") == 0) return false;
    if (strcmp(value, "false") == 0) return false;
    return true;
}

/* Helper function to find matching block end tag */
static char* tct_find_block_end(char *start, const char *start_tag, const char *end_tag) {
    size_t start_len = strlen(start_tag);
    size_t end_len = strlen(end_tag);
    int depth = 1;
    char *pos = start;
    
    while (*pos && depth > 0) {
        if (memcmp(pos, TCT_START_SIGN, TCT_START_SIGN_LEN) == 0) {
            char *tag_start = pos + TCT_START_SIGN_LEN;
            /* Skip whitespace */
            while (*tag_start && (*tag_start == ' ' || *tag_start == '\t')) tag_start++;
            
            if (memcmp(tag_start, start_tag, start_len) == 0) {
                depth++;
            } else if (memcmp(tag_start, end_tag, end_len) == 0) {
                depth--;
                if (depth == 0) {
                    return pos;
                }
            }
        }
        pos++;
    }
    return NULL;
}

/* Helper function to find else block within an if block */
static char* tct_find_else_block(char *start, char *end) {
    char *pos = start;
    int depth = 0;
    
    while (pos < end) {
        if (memcmp(pos, TCT_START_SIGN, TCT_START_SIGN_LEN) == 0) {
            char *tag_start = pos + TCT_START_SIGN_LEN;
            /* Skip whitespace */
            while (tag_start < end && (*tag_start == ' ' || *tag_start == '\t')) tag_start++;
            
            if (memcmp(tag_start, "#if ", 4) == 0 || memcmp(tag_start, "#each ", 6) == 0) {
                depth++;
            } else if (memcmp(tag_start, "/if", 3) == 0 || memcmp(tag_start, "/each", 5) == 0) {
                depth--;
            } else if (depth == 0 && memcmp(tag_start, "#else", 5) == 0) {
                return pos;
            }
        }
        pos++;
    }
    return NULL;
}
char* tct_render(char *template, tct_arguments *argument) {
#define IS_WHITESPACE(c) (c==' ' || c=='\t' || c=='\r' || c=='\n') 
    tct_section *section_start, *section_current;
    char *start, *end;

    char *result, *write;
    size_t result_len;

    section_start = calloc(1, sizeof(tct_section));
    section_current = section_start;

    result_len = strlen(template);
    while (tct_find_symbol(template, &start, &end)) {
        char *trim_start, *trim_end;

        section_current->data = template;
        section_current->should_free = false;
        section_current->length = start - template;
        result_len += section_current->length;
        section_current->next = calloc(1, sizeof(tct_section));
        section_current = section_current->next;

        trim_start = start + TCT_START_SIGN_LEN;
        trim_end = end;
        while (IS_WHITESPACE(trim_start[0])) trim_start++;
        while (IS_WHITESPACE(trim_end[-1])) trim_end--;
        
        /* Check for conditional blocks: {{#if variable}} */
        if (memcmp(trim_start, "#if ", 4) == 0) {
            char *var_name = trim_start + 4;
            while (IS_WHITESPACE(*var_name)) var_name++;
            size_t var_len = trim_end - var_name;
            
            char *block_end = tct_find_block_end(end + TCT_END_SIGN_LEN, "#if ", "/if");
            if (block_end) {
                char *else_pos = tct_find_else_block(end + TCT_END_SIGN_LEN, block_end);
                const char *value = tct_get_valuen(argument, var_name, var_len);
                
                if (tct_is_truthy(value)) {
                    /* Render the if block */
                    char *if_content = end + TCT_END_SIGN_LEN;
                    size_t if_len = (else_pos ? else_pos : block_end) - if_content;
                    char *if_template = malloc(if_len + 1);
                    memcpy(if_template, if_content, if_len);
                    if_template[if_len] = '\0';
                    
                    char *rendered = tct_render(if_template, argument);
                    section_current->data = rendered;
                    section_current->should_free = true;
                    section_current->length = strlen(rendered);
                    result_len += section_current->length;
                    free(if_template);
                } else if (else_pos) {
                    /* Render the else block */
                    char *else_start = else_pos;
                    while (*else_start && memcmp(else_start, TCT_END_SIGN, TCT_END_SIGN_LEN) != 0) else_start++;
                    else_start += TCT_END_SIGN_LEN;
                    
                    size_t else_len = block_end - else_start;
                    char *else_template = malloc(else_len + 1);
                    memcpy(else_template, else_start, else_len);
                    else_template[else_len] = '\0';
                    
                    char *rendered = tct_render(else_template, argument);
                    section_current->data = rendered;
                    section_current->should_free = true;
                    section_current->length = strlen(rendered);
                    result_len += section_current->length;
                    free(else_template);
                }
                
                section_current->next = calloc(1, sizeof(tct_section));
                section_current = section_current->next;
                
                /* Skip past the closing tag */
                template = block_end;
                while (*template && memcmp(template, TCT_END_SIGN, TCT_END_SIGN_LEN) != 0) template++;
                if (*template) template += TCT_END_SIGN_LEN;
                continue;
            }
        }
        /* Check for loop blocks: {{#each variable}} */
        else if (memcmp(trim_start, "#each ", 6) == 0) {
            char *var_name = trim_start + 6;
            while (IS_WHITESPACE(*var_name)) var_name++;
            size_t var_len = trim_end - var_name;
            
            char *block_end = tct_find_block_end(end + TCT_END_SIGN_LEN, "#each ", "/each");
            if (block_end) {
                /* Collect all arguments with this name into an array for reverse iteration */
                tct_arguments **arg_array = NULL;
                int arg_count = 0;
                int arg_capacity = 0;
                
                tct_arguments *current_arg = tct_find_arguments(argument, var_name, var_len);
                while (current_arg) {
                    if (arg_count >= arg_capacity) {
                        arg_capacity = arg_capacity ? arg_capacity * 2 : 4;
                        tct_arguments **new_array = realloc(arg_array, arg_capacity * sizeof(tct_arguments*));
                        if (!new_array) {
                            /* Out of memory - free what we have and skip this loop */
                            free(arg_array);
                            break;
                        }
                        arg_array = new_array;
                    }
                    arg_array[arg_count++] = current_arg;
                    current_arg = tct_find_next_argument(current_arg, var_name, var_len);
                }
                
                /* Iterate in reverse order (to match the order items were added) */
                for (int i = arg_count - 1; i >= 0; i--) {
                    const char *value = &arg_array[i]->data[var_len + 1];
                    size_t value_len = strlen(value);
                    
                    /* Only render if value is truthy */
                    if (tct_is_truthy(value)) {
                        char *loop_content = end + TCT_END_SIGN_LEN;
                        size_t loop_len = block_end - loop_content;
                        char *loop_template = malloc(loop_len + 1);
                        memcpy(loop_template, loop_content, loop_len);
                        loop_template[loop_len] = '\0';
                        
                        /* Create a scoped argument with current value prepended
                         * This ensures {{ variable }} inside the loop refers to the current iteration value */
                        tct_arguments *scoped_args = calloc(1, sizeof(tct_arguments) + var_len + 1 + value_len + 1);
                        memcpy(scoped_args->data, var_name, var_len);
                        scoped_args->data[var_len] = '\0';
                        strcpy(&scoped_args->data[var_len + 1], value);
                        scoped_args->next = argument;
                        
                        char *rendered = tct_render(loop_template, scoped_args);
                        section_current->data = rendered;
                        section_current->should_free = true;
                        section_current->length = strlen(rendered);
                        result_len += section_current->length;
                        free(loop_template);
                        free(scoped_args);
                        
                        section_current->next = calloc(1, sizeof(tct_section));
                        section_current = section_current->next;
                    }
                }
                
                if (arg_array) {
                    free(arg_array);
                }
                
                /* Skip past the closing tag */
                template = block_end;
                while (*template && memcmp(template, TCT_END_SIGN, TCT_END_SIGN_LEN) != 0) template++;
                if (*template) template += TCT_END_SIGN_LEN;
                continue;
            }
        }
        /* Check for closing tags - skip them as they're handled by the opening tags */
        else if (memcmp(trim_start, "/if", 3) == 0 || memcmp(trim_start, "/each", 5) == 0 || 
                 memcmp(trim_start, "#else", 5) == 0) {
            template = end + TCT_END_SIGN_LEN;
            continue;
        }
        
        /* Default: variable substitution */
        section_current->data = tct_get_valuen(argument, trim_start, trim_end - trim_start);
        section_current->should_free = false;
        section_current->length = strlen(section_current->data);
        result_len += section_current->length;
        section_current->next = calloc(1, sizeof(tct_section));
        section_current = section_current->next;

        template = end + TCT_END_SIGN_LEN;
    }
    section_current->data = template;
    section_current->should_free = false;
    section_current->length = strlen(template);

    result = malloc(result_len + 1);
    write = result;

    while (section_start) {
        tct_section *section = section_start;
        memcpy(write, section->data, section->length);
        write += section->length;
        section_start = section->next;
        if (section->should_free && section->data) free(section->data);
        free(section);
    }
    *write = 0;

    return result;
#undef IS_WHITESPACE
}
