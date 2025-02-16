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
    HARGPARSE_ARG_LINKED next = handle->arguments;
    while (next)
    {
        next = free_linked_argument(next);
    }
    free (handle);
}

ARG_ARGUMENT_HANDLE argsparse_create_argument_with_value(ARG_TYPE type, const char* name, const char* desc, void* value)
{
    ARG_ARGUMENT_HANDLE p = create_argument(name, desc);
    p->type = type;
    parse_value(&p->value, type, value);
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
/// @param argument Handle to allocated argument
ARG_ERROR argsparse_put_argument(ARG_DATA_HANDLE handle, ARG_ARGUMENT_HANDLE* href)
{
    if (handle->count >= ARGSPARSE_MAX_ARGUMENTS)
    {
        // didn't take ownership
        return ERROR_MAX_ARGUMENTS;
    }
    handle->count++;
    ARG_ARGUMENT_HANDLE arg = *href;
    HARGPARSE_ARG_LINKED new_link = calloc(1, sizeof(t_argparse_argument_linked));
    new_link->argument = arg;
    generate_short_name(handle, arg);
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
    // take ownership
    *href = NULL;
    return ERROR_NONE;
}

void argsparse_add_int(ARG_DATA_HANDLE handle, const char* option, const char* description, int value)
{

}

void argsparse_add_double(ARG_DATA_HANDLE handle, const char* option, const char* description, double value)
{

}

void AddArgumentTypeOnOff(ARG_DATA_HANDLE handle, const char* option, const char* description, const char* value)
{

}

void argsparse_add_str(ARG_DATA_HANDLE handle, const char* option, const char* description, const char* value)
{

}

int argsparse_add_flag(ARG_DATA_HANDLE handle, const char* name, const char* description, int value)
{
    ARG_ARGUMENT_HANDLE p = argsparse_create_argument_with_value(ARGSPARSE_TYPE_FLAG, name, description, (void*)(uintptr_t)value);

    int err = argsparse_put_argument(handle, &p);

    if (p != NULL)
    {
        free_argument(&p);
    }
    
    return err;
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
        iterate_arguments_return_on_zero(handle, action_do_option_long, (void*)long_options);
        int c;
        while(1)
        {
            c = getopt_long(argc, argv,
                            handle->shortopts,
                            (const struct option *)long_options,
                            &option_index);

            /* Detect the end of the options. */
            if (c == -1)
                break;

            /* opt->flag */
            if (c == 0)
            {
                count++;
                continue;
            }

            ARG_ARGUMENT_HANDLE arg = argsparse_argument_by_short_name(handle, c);
            if (arg == NULL)
            {
                printf ("invalid option -%c", c);
                argsparse_usage(handle);
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
        }
        free(long_options);
        iterate_arguments_return_on_zero(handle, action_mark_parsed_flags, NULL);
    }
    return count;
}

void argsparse_usage(ARG_DATA_HANDLE handle)
{

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

static void free_argument(ARG_ARGUMENT_HANDLE* handle)
{
    free(*handle);
    *handle = NULL;
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
