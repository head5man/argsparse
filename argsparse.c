/**
 * @file argsparse.c
 * @author Tuomas Lahtinen (tuomas123lahtinen@gmail.com)
 * @brief Generic command line arguments definition utility
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "iterate.h"
#include "internal_funcs.h"

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <getopt.h>

ARG_DATA_HANDLE argsparse_create(const char* title) 
{
    ARG_DATA_HANDLE p = calloc(1, sizeof(argument_data_t));
    p->title = title;
    p->arguments = NULL;
    return p;
}

void argsparse_free(ARG_DATA_HANDLE handle)
{
    if (handle)
    {
        HARGPARSE_ARG_LINKED next = handle->arguments;
        while (next)
        {
            next = free_linked_argument(next);
        }
        free (handle);
    }
}

ARG_ARGUMENT_HANDLE argsparse_create_argument_with_value(ARG_TYPE type, const char* name, const char* desc, ARG_VALUE* value)
{
    ARG_ARGUMENT_HANDLE p = create_argument(name, desc);
    p->type = type;
    if (value)
        memcpy(&p->value, value, sizeof(ARG_VALUE));
    return p;
}

char* argsparse_get_shortopts(ARG_DATA_HANDLE handle)
{
    return (handle) ? handle->shortopts : NULL;
}

const char* argsparse_get_title(ARG_DATA_HANDLE handle)
{
    return (handle) ? handle->title : NULL;
}

ARG_ARGUMENT_HANDLE argsparse_argument_by_name(ARG_DATA_HANDLE handle, const char* name)
{
    return iterate_arguments_return_on_zero(handle, predicate_compare_name, (void*)name);
}

ARG_ARGUMENT_HANDLE argsparse_argument_by_short_name(ARG_DATA_HANDLE handle, int shortname)
{
    return iterate_arguments_return_on_zero(handle, predicate_compare_short_name, (void*)&shortname);
}

int argsparse_argument_count(ARG_DATA_HANDLE handle)
{
    int ret = 0;
    iterate_arguments_return_on_zero(handle, action_count, &ret);
    return ret;
}

/// @brief Add argument moves argument ownership to handle
/// @param handle Handle to allocated arguments structure
/// @param href argument to take ownership of
ARG_ERROR argsparse_put_argument(ARG_DATA_HANDLE handle, ARG_ARGUMENT_HANDLE* href)
{
    if (handle->count >= ARGSPARSE_MAX_ARGS)
    {
        free_argument(href);
        return ERROR_MAX_ARGS;
    }
    handle->count++;
    HARGPARSE_ARG_LINKED new_link = calloc(1, sizeof(t_argparse_argument_linked));
    new_link->argument = *href;
    *href = NULL;

    generate_short_name(handle, new_link->argument);
    if (handle->arguments)
    {
        HARGPARSE_ARG_LINKED next = handle->arguments;
        while (next->next)
        {
            next = next->next;
        }
        next->next = new_link;
    }
    else
        handle->arguments = new_link;

    return ERROR_NONE;
}

ARG_ERROR argsparse_add_int(ARG_DATA_HANDLE handle, const char* name, const char* description, int value)
{
    ARG_VALUE argvalue;
    argvalue.intvalue = value;

    ARG_ARGUMENT_HANDLE p = argsparse_create_argument_with_value(ARGSPARSE_TYPE_INT, name, description, &argvalue);

    return argsparse_put_argument(handle, &p);
}

ARG_ERROR argsparse_add_double(ARG_DATA_HANDLE handle, const char* name, const char* description, double value)
{
    ARG_VALUE argvalue;
    argvalue.doublevalue = value;

    ARG_ARGUMENT_HANDLE p = argsparse_create_argument_with_value(ARGSPARSE_TYPE_DOUBLE, name, description, &argvalue);

    return argsparse_put_argument(handle, &p);
}

ARG_ERROR argsparse_add_cstr(ARG_DATA_HANDLE handle, const char* name, const char* description, const char* value)
{
    ARG_VALUE argvalue;
    copy_to_argument_string(argvalue.stringvalue, value);

    ARG_ARGUMENT_HANDLE p = argsparse_create_argument_with_value(ARGSPARSE_TYPE_STRING, name, description, &argvalue);
    return argsparse_put_argument(handle, &p);
}

ARG_ERROR argsparse_add_flag(ARG_DATA_HANDLE handle, const char* name, const char* description, int value)
{
    ARG_VALUE argvalue;
    argvalue.flagvalue = value == 0 ? 0 : 1;

    ARG_ARGUMENT_HANDLE p = argsparse_create_argument_with_value(ARGSPARSE_TYPE_FLAG, name, description, &argvalue);

    return argsparse_put_argument(handle, &p);
}

int argsparse_parse_args(ARG_DATA_HANDLE handle, char* const *argv, int argc)
{
    int count = 0;
    if (argc > 1)
    {
        struct option *long_options = calloc(argc, sizeof(struct option));
            // {
            //   /* These options set a flag. */
            //   {"verbose", no_argument,       &verbose_flag, 1},
            //   {"brief",   no_argument,       &verbose_flag, 0},
            //   /* These options don’t set a flag.
            //      We distinguish them by their indices. */
            //   {"file",    required_argument, 0, 'f'},
            //   {0, 0, 0, 0}
            // };
        /* getopt_long stores the option index here. */
        int option_index = 0;
        optind = 0;
        iterate_arguments_return_on_zero(handle, action_do_option_long, (void*)(long_options + 1));
        int c = 0;
        while(c != -1)
        {
            c = getopt_long(argc, argv,
                            handle->shortopts,
                            (const struct option *)long_options,
                            &option_index);


            switch (c)
            {
                /* Detect the end of the options. */
                case -1:
                    continue;

                /* opt->flag */
                case 0:
                    count++;
                    continue;

                case 'h':
                    argsparse_usage(handle, argv[0]);
                    exit(0);

                default:
                    ARG_ARGUMENT_HANDLE arg = argsparse_argument_by_short_name(handle, c);
                    if (arg == NULL)
                    {
                        printf ("invalid option -%c\n", c);
                        argsparse_usage(handle, argv[0]);
                        exit(1);
                    }
                    else
                    {
                        int err = parse_value(&arg->value, arg->type, optarg);
                        if (!err)
                        {
                            arg->parsed = 1;
                            count++;
                        }
                    }
                    break;
            }
        }
        free(long_options);
        iterate_arguments_return_on_zero(handle, action_mark_parsed_flags, NULL);
    }
    return count;
}

void argsparse_usage(ARG_DATA_HANDLE handle, const char* const executable)
{
    // is there a separator?
    const char* separator = strrchr(executable, '/') ? strrchr(executable, '/') : strrchr(executable, '\\');
    // advance or fallback to executable
    const char* basename = separator ? separator + 1 : executable;

    printf("usage: %s", basename);
    char* shortopt = handle->shortopts;
    while (*shortopt)
    {
        char c = *shortopt;
        shortopt++;
        if (c == ':')
            continue;

        printf(" [-%c]", c);
    }
    printf("\n%s\n", handle->title);

    printf("optional arguments:\n");
    size_t width = 0;
    iterate_arguments_return_on_zero(handle, action_long_option_width, &width);
    iterate_arguments_return_on_zero(handle, action_argument_usage, (void*)(uintptr_t)width);
}

////////////////////////
// Internal functions //
////////////////////////

static ARG_ARGUMENT_HANDLE create_argument(const char* name, const char* desc)
{
    ARG_ARGUMENT_HANDLE p = calloc(1, sizeof(argsparse_argument_t));
    copy_to_argument_string(p->name, name);
    copy_to_argument_string(p->description, desc);
    return p;
}

static HARGPARSE_ARG_LINKED free_linked_argument(HARGPARSE_ARG_LINKED linked)
{
    HARGPARSE_ARG_LINKED next = NULL;
    if (linked != NULL)
    {
        next = linked->next;
        linked->next = NULL;
        ARG_ARGUMENT_HANDLE* arg = &(linked->argument);
        free_argument(arg);
        free(linked);
    }
    return next;
}

/// @brief free argument and null
/// @param handle null-safe
static void free_argument(ARG_ARGUMENT_HANDLE* handle)
{
    if (*handle)
    {
        free(*handle);
        *handle = NULL;
    }
}

////////////////////////////////////
// Iterate actions and predicates //
////////////////////////////////////

static ARG_ARGUMENT_HANDLE iterate_arguments_return_on_zero(ARG_DATA_HANDLE handle, int(*predicate)(int, ARG_ARGUMENT_HANDLE, void*), void* data)
{
    ARG_ARGUMENT_HANDLE ret = NULL;
    HARGPARSE_ARG_LINKED iterator = ((argument_data_t*)handle)->arguments;
    int idx = 0;
    while (iterator && predicate)
    {
        ARG_ARGUMENT_HANDLE argument = iterator->argument;
        // call predicate and return when zero
        if ((*predicate)(idx, argument, data) == 0)
        {
            ret = argument;
            break;
        }
        idx++;
        iterator = iterator->next;
    }
    return ret;
}

static int action_do_option_long(int idx, ARG_ARGUMENT_HANDLE arg, void* data)
{
    struct option* options = (struct option*)data;
    options[idx].name = arg->name;
    options[idx].has_arg = arg->type != ARGSPARSE_TYPE_NONE && arg->type != ARGSPARSE_TYPE_FLAG;
    options[idx].val = arg->name_short;
    options[idx].flag = (arg->type == ARGSPARSE_TYPE_FLAG) ? &(arg->value.flagvalue) : NULL;
    return 1;
}

static int action_long_option_width(int idx, ARG_ARGUMENT_HANDLE arg, void* data)
{
    size_t width = strlen(arg->name);
    size_t* ret = (size_t*)(uintptr_t)data;
    *ret = width > *ret ? width : *ret;
    return 1;
}

static int action_argument_usage(int idx, ARG_ARGUMENT_HANDLE arg, void* data)
{
    char buffer[ARGSPARSE_MAX_STRING_SIZE] = {0,};
    int width = (int)(uintptr_t)data;
    //-<shortopt>, --<longopt> [argtype] <description>
    printf("-%c, ", arg->name_short);
    printf("--%-*s ", width, arg->name);
    printf("[%s] %s\n", get_expected_argument_type(arg->type), arg->description);
    printf("  def: %s\n", get_argument_value_string(arg, buffer, ARGSPARSE_MAX_STRING_SIZE));
    return 1;
}

static int action_count(int idx, ARG_ARGUMENT_HANDLE arg, void* data)
{
    int* p = (int*)(uintptr_t)data;
    int value = *p;
    *p = value + 1;
    return 1;
}

static int action_mark_parsed_flags(int idx, ARG_ARGUMENT_HANDLE arg, void* data)
{
    if (arg && arg->type == ARGSPARSE_TYPE_FLAG)
    {
        // flag value was set by getopts_long
        if (arg->value.flagvalue == arg->name_short)
        {
            arg->parsed = 1;
            arg->value.flagvalue = 1;
        }
    }
    return 1;
}

static int predicate_compare_name(int idx, ARG_ARGUMENT_HANDLE arg, void* data)
{
    const char* name = (const char*)data;
    return strcmp(name, arg->name);
}

static int predicate_compare_short_name(int idx, ARG_ARGUMENT_HANDLE arg, void* data)
{
    int name = *((int*)data);
    return (name == arg->name_short) ? 0 : 1;
}
