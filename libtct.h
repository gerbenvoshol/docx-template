#ifndef LIBTCT_H
#define LIBTCT_H

#include <stddef.h>
#include <stdarg.h>

/**
 * libtct - Tiny C Template engine
 * 
 * A micro template engine for C with support for:
 * - Variable substitution: {{ variable_name }}
 * - Conditional blocks: {{#if condition}} ... {{/if}}
 * - Conditional else: {{#if condition}} ... {{#else}} ... {{/if}}
 * - Loop blocks: {{#each items}} ... {{/each}}
 * - Nested templates
 * 
 * Template delimiters can be customized by modifying TCT_START_SIGN and TCT_END_SIGN
 */

#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif

typedef enum {false_val=0, true_val=1} bool;

/* Template delimiters */
#define TCT_START_SIGN "{{"
#define TCT_END_SIGN "}}"
#define TCT_START_SIGN_LEN (sizeof(TCT_START_SIGN)-1)
#define TCT_END_SIGN_LEN (sizeof(TCT_END_SIGN)-1)

/**
 * tct_arguments - Linked list structure for template arguments
 * 
 * Stores name-value pairs for template variable substitution.
 * The data array contains: [name\0value\0]
 */
typedef struct _tct_arguments {
    struct _tct_arguments *next;
    char data[0];  /* Flexible array member: [name\0value\0] */
} tct_arguments;

/**
 * tct_section - Internal linked list for template sections
 * 
 * Used internally during rendering to build the output string
 */
typedef struct _tct_section {
    char *data;
    size_t length;
    bool should_free;  /* Whether data should be freed */
    struct _tct_section *next;
} tct_section;

/**
 * tct_add_argument_ - Add a new argument to the arguments list
 * 
 * @param next_argument: Next argument in the list (or NULL)
 * @param name: Variable name
 * @param format: Printf-style format string
 * @param ...: Format arguments
 * 
 * Returns: New argument (becomes the new head of the list)
 * 
 * Note: Use the tct_add_argument() macro for easier syntax
 */
tct_arguments* tct_add_argument_(tct_arguments *next_argument, char *name, const char *format, ...);

/**
 * tct_add_argument - Macro to add an argument to the list
 * 
 * Usage: tct_add_argument(args, "name", "%s", "value");
 * 
 * This macro modifies the first argument in place, making it point to the new head
 */
#define tct_add_argument(a, ...) {a = tct_add_argument_(a, __VA_ARGS__);}

/**
 * tct_find_arguments - Find an argument by name
 * 
 * @param arguments: Head of arguments list
 * @param name: Variable name to find
 * @param name_len: Length of the name string
 * 
 * Returns: Pointer to matching argument or NULL if not found
 */
tct_arguments* tct_find_arguments(tct_arguments *arguments, char *name, size_t name_len);

/**
 * tct_find_argument - Macro to find an argument by name (calculates length)
 * 
 * Usage: tct_arguments *arg = tct_find_argument(args, "name");
 */
#define tct_find_argument(a, n) tct_find_arguments(a, n, strlen(n))

/**
 * tct_get_valuen - Get the value of an argument by name
 * 
 * @param arguments: Head of arguments list
 * @param name: Variable name
 * @param name_len: Length of the name string
 * 
 * Returns: Pointer to value string, or empty string "" if not found
 */
char* tct_get_valuen(tct_arguments *arguments, char *name, size_t name_len);

/**
 * tct_get_value - Macro to get an argument value by name (calculates length)
 * 
 * Usage: char *value = tct_get_value(args, "name");
 */
#define tct_get_value(a, n) tct_get_valuen(a, n, strlen(n))

/**
 * tct_free_argument - Free all arguments in the list
 * 
 * @param arguments: Head of arguments list
 * 
 * Frees all memory allocated for the arguments list
 */
void tct_free_argument(tct_arguments* arguments);

/**
 * tct_render - Render a template with the given arguments
 * 
 * @param template: Template string with {{ }} placeholders
 * @param argument: Arguments list for substitution
 * 
 * Returns: Newly allocated string with rendered output (caller must free)
 * 
 * Supported template syntax:
 * - {{ variable_name }} - Variable substitution
 * - {{#if var}} ... {{/if}} - Conditional block (shows if var is non-empty)
 * - {{#if var}} ... {{#else}} ... {{/if}} - Conditional with else
 * - {{#each var}} ... {{/each}} - Loop block (repeats for each item)
 */
char* tct_render(char *template, tct_arguments *argument);

#endif /* LIBTCT_H */
