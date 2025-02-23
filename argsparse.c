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

ARG_ERROR argsparse_add(ARG_DATA_HANDLE handle, const char* name, const char* description, ARG_TYPE type, const ARG_VALUE* value)
{
    ARG_ARGUMENT_HANDLE h = create_argument(type, name, description, value);
    return put_argument(handle, &h);
}

ARG_ERROR argsparse_add_help(ARG_DATA_HANDLE handle)
{
    ARG_ARGUMENT_HANDLE p = create_argument(ARGSPARSE_TYPE_NONE, "help", "Print this message", NULL);
    return put_argument(handle, &p);
}

ARG_ERROR argsparse_add_int(ARG_DATA_HANDLE handle, const char* name, const char* description, int value)
{
    ARG_VALUE argvalue;
    argvalue.intvalue = value;

    ARG_ARGUMENT_HANDLE p = create_argument(ARGSPARSE_TYPE_INT, name, description, &argvalue);

    return put_argument(handle, &p);
}

ARG_ERROR argsparse_add_double(ARG_DATA_HANDLE handle, const char* name, const char* description, double value)
{
    ARG_VALUE argvalue;
    argvalue.doublevalue = value;

    ARG_ARGUMENT_HANDLE p = create_argument(ARGSPARSE_TYPE_DOUBLE, name, description, &argvalue);

    return put_argument(handle, &p);
}

ARG_ERROR argsparse_add_cstr(ARG_DATA_HANDLE handle, const char* name, const char* description, const char* value)
{
    ARG_VALUE argvalue;
    copy_to_argument_string(argvalue.stringvalue, value);

    ARG_ARGUMENT_HANDLE p = create_argument(ARGSPARSE_TYPE_STRING, name, description, &argvalue);
    return put_argument(handle, &p);
}

ARG_ERROR argsparse_add_flag(ARG_DATA_HANDLE handle, const char* name, const char* description, int value, int* ptr_to_value)
{
    ARG_VALUE argvalue = {0, };
    argvalue.flagptr = ptr_to_value;
    ARG_ARGUMENT_HANDLE p = create_argument(ARGSPARSE_TYPE_FLAG, name, description, &argvalue);
    p->flag_init.flagvalue = value;
    return put_argument(handle, &p);
}

int argsparse_parse_args(ARG_DATA_HANDLE handle, char* const *argv, int argc)
{
    int count = 0;
    // Although application call this function only once,
    // tests call this function many times and optind has to be reset.
    // https://github.com/skandhurkat/Getopt-for-Visual-Studio/blob/6567b18432b1b4dc0e71f71b8601df28c1ac09f8/getopt.h#L80
    optind = 1;
    if (argc > 1)
    {
        int arg_count = argsparse_argument_count(handle);
        struct option *long_options = calloc(arg_count + 1, sizeof(struct option));

        if (long_options != NULL)
        {
            int c = 0;
            iterate_arguments_return_on_zero(handle, action_do_option_long, (void*)(long_options));
            while(c != -1)
            {
                /* getopt_long stores the option index here (long_options[option_index]). */
                int option_index = 0;
                c = getopt_long(argc, argv,
                                handle->shortopts,
                                (const struct option *)long_options,
                                &option_index);
                printf("option_index(%d), optind(%d)\n", option_index, optind);

                switch (c)
                {
                    /* Detect the end of the options. */
                    case -1:
                        break;

                    /* opt->flag */
                    case 0:
                        printf ("flag -%c\n", c);
                        count++;
                        break;

                    case 'h':
                        argsparse_show_usage(handle, argv[0]);
                        exit(0);

                    default:
                        printf ("option -%c\n", c);
                        ARG_ARGUMENT_HANDLE arg = argsparse_argument_by_short_name(handle, c);
                        if (arg == NULL)
                        {
                            printf ("invalid option -%c\n", c);
                            argsparse_show_usage(handle, argv[0]);
                            exit(1);
                        }
                        else
                        {
                            int err = parse_value(&arg->value, arg->type, optarg);
                            if (!err)
                            {
                                printf("parsed %s\n", optarg);
                                arg->parsed = 1;
                                count++;
                            }
                        }
                        break;
                }
            }

            if (optind < argc) {
                printf("non-option ARGV-elements: ");
                while (optind < argc)
                    printf("%s ", argv[optind++]);

                printf("\n");
            }
            iterate_arguments_return_on_zero(handle, action_mark_parsed_flags, NULL);
            free(long_options);
        }
    }
    return count;
}

void argsparse_show_usage(ARG_DATA_HANDLE handle, const char* const executable)
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
    printf("\ntitle: %s\n", handle->title);

    printf("optional arguments:\n");

    iterate_arguments_return_on_zero(handle, action_show_argument_usage, NULL);
}

void argsparse_show_arguments(ARG_DATA_HANDLE handle)
{
    printf("argument values:\n");
    size_t width = 0;
    iterate_arguments_return_on_zero(handle, action_long_option_width, &width);
    iterate_arguments_return_on_zero(handle, action_show_argument_value, (void*)(uintptr_t)width);
}

////////////////////////
// Internal functions //
////////////////////////

static ARG_ARGUMENT_HANDLE create_argument(ARG_TYPE type, const char* name, const char* desc, const ARG_VALUE* value)
{
    ARG_ARGUMENT_HANDLE p = calloc(1, sizeof(argsparse_argument_t));
    copy_to_argument_string(p->name, name);
    copy_to_argument_string(p->description, desc);

    p->type = type;
    if (value)
    {
        memcpy(&p->value, value, sizeof(ARG_VALUE));
        // if no pointer given point to the unused ARG_VALUE memory
        if (type == ARGSPARSE_TYPE_FLAG && p->value.flagptr == NULL)
        {
            p->value.flagptr = (int*)(&p->value.flagptr + 1);
        }
    }
    return p;
}

static ARG_ERROR put_argument(ARG_DATA_HANDLE handle, ARG_ARGUMENT_HANDLE* href)
{
    ARG_ARGUMENT_HANDLE exists = iterate_arguments_return_on_zero(handle, predicate_compare_name, (*href)->name);
    if (exists)
    {
        // free if not the same
        if (exists != *href)
        {
            free_argument(href);
        }
        return ERROR_EXISTS;
    }

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
    options[idx].val = (arg->type == ARGSPARSE_TYPE_FLAG) ? arg->flag_init.flagvalue : arg->name_short;
    options[idx].flag = (arg->type == ARGSPARSE_TYPE_FLAG) ? arg->value.flagptr : NULL;
    return 1;
}

static int action_long_option_width(int idx, ARG_ARGUMENT_HANDLE arg, void* data)
{
    size_t width = strlen(arg->name);
    size_t* ret = (size_t*)(uintptr_t)data;
    *ret = width > *ret ? width : *ret;
    return 1;
}

static int action_show_argument_usage(int idx, ARG_ARGUMENT_HANDLE arg, void* data)
{
    char buffer[ARGSPARSE_MAX_STRING_SIZE] = {0,};
    printf("-%c, ", arg->name_short);
    printf("--%s\n", arg->name);
    printf("    desc: %s\n", arg->description);
    if (arg->type != ARGSPARSE_TYPE_NONE)
    {
        printf("    args: [%s", get_argument_type_string(arg->type));
        printf(":%s", get_argument_value_string(arg, buffer, ARGSPARSE_MAX_STRING_SIZE));
        printf("]\n");
    }
    printf("\n");
    return 1;
}

static int action_show_argument_value(int idx, ARG_ARGUMENT_HANDLE arg, void* data)
{
    char buffer[ARGSPARSE_MAX_STRING_SIZE] = {0,};
    if (arg->type != ARGSPARSE_TYPE_NONE)
    {
        size_t width = (uintptr_t)(data);
        if (width != 0 && width < 20)
        {
            printf("    %*s: ", (int)width, arg->name);
        }
        else
        {
            printf("    %s: ", arg->name);
        }

        printf("[%s]", get_argument_type_string(arg->type));
        printf(" %s", get_argument_value_string(arg, buffer, ARGSPARSE_MAX_STRING_SIZE));
        printf("\n");
    }
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
        if (*arg->value.flagptr == arg->flag_init.flagvalue)
        {
            arg->parsed = 1;
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
