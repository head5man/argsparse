/**
 * @file argsparse.c
 * @author Tuomas Lahtinen (tuomas123lahtinen@gmail.com)
 * @brief Generic command line arguments definition utility
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "argsparse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <getopt.h>

typedef struct _argparse_argument_linked
{
    HARGPARSE_ARG argument;
    void* next;
} t_argparse_argument_linked;

typedef struct _argparse_argument_linked* HARGPARSE_ARG_LINKED;
typedef struct _argparse_struct
{
    char shortopts[ARGSPARSE_MAX_ARGUMENTS * 2];
    int count;
    HARGPARSE_ARG_LINKED arguments;
    const char* title;
} t_argparse_struct;

static HARGPARSE_ARG_LINKED delete_argument(HARGPARSE_ARG_LINKED arg);
static int parse_value(HARGPARSE_VALUE* ref, tArgsparseValuetype type, const char* value);

HARGPARSE_HANDLE argsparse_create(const char* title) 
{
    HARGPARSE_HANDLE p = calloc(1, sizeof(t_argparse_struct));
    p->title = title;
    p->arguments = NULL;
}

void argsparse_delete(HARGPARSE_HANDLE handle)
{
    HARGPARSE_ARG_LINKED next = handle->arguments;
    while (next)
    {
        next = delete_argument(next);
    }
    free (handle);
}

HARGPARSE_ARG argsparse_argument_allocate(const char* name, const char* desc)
{
    HARGPARSE_ARG p = calloc(1, sizeof(t_argparse_argument));
    p->name = strdup(name);
    p->description = strdup(desc);
    return p;
}

HARGPARSE_ARG argsparse_create_argument(tArgsparseValuetype type, const char* name, const char* desc, const char* value)
{
    HARGPARSE_ARG p = argsparse_argument_allocate(name, desc);
    p->type = type;
    parse_value(&p->value, type, value);
    return p;
}

char* argsparse_get_shortopts(HARGPARSE_HANDLE handle)
{
    return (handle) ? handle->shortopts : NULL;
}

const char* argsparse_get_title(HARGPARSE_HANDLE handle)
{
    return (handle) ? handle->title : NULL;
}

static int argument_compare_name(HARGPARSE_ARG arg, void* data)
{
    const char* name = (const char*)data;
    return strcmp(name, arg->name);
}

static int argument_compare_short_name(HARGPARSE_ARG arg, void* data)
{
    int name = *((int*)data);
    return (name == arg->name_short) ? 0 : 1;
}

HARGPARSE_ARG argsparse_argument_get(HARGPARSE_HANDLE handle, int(*cmp)(HARGPARSE_ARG, void*), void* cmpdata)
{
    HARGPARSE_ARG ret = NULL;
    HARGPARSE_ARG_LINKED linked = ((t_argparse_struct*)handle)->arguments;
    while (linked && cmp)
    {
        HARGPARSE_ARG argument = linked->argument;
        if ((*cmp)(argument, cmpdata) == 0)
        {
            ret = argument;
            break;
        }
        linked = linked->next;
    }
    return ret;
}

HARGPARSE_ARG argsparse_argument_by_name(HARGPARSE_HANDLE handle, const char* name)
{
    return argsparse_argument_get(handle, argument_compare_name, (void*)name);
}

HARGPARSE_ARG argsparse_argument_by_short_name(HARGPARSE_HANDLE handle, int shortname)
{
    return argsparse_argument_get(handle, argument_compare_short_name, (void*)&shortname);
}

static void do_option_long(int idx, HARGPARSE_ARG arg, void* data)
{
    struct option* options = (struct option*)data;
    options[idx].name = arg->name;
    options[idx].has_arg = arg->type != ARGSPARSE_TYPE_NONE;
    options[idx].val = arg->name_short;
}

int argsparse_argument_action(HARGPARSE_HANDLE handle, void (*action)(int, HARGPARSE_ARG, void*), void* actiondata)
{
    int ret = 0;
    t_argparse_argument_linked* iterator = ((t_argparse_struct*)handle)->arguments;
    while (iterator)
    {
        // invoke caller action
        if (action)
            (*action)(ret, iterator->argument, actiondata);
        ret++;
        iterator = iterator->next;
    }
    return ret;
}

int argsparse_argument_count(HARGPARSE_HANDLE handle)
{
    return argsparse_argument_action(handle, NULL, NULL);
}

static const char* FindEndTerminator(const char* str)
{
    const char* end = NULL;
    char terminators[] = {' ', '\0'};
    for (int i = 0; str && end == NULL && i < sizeof(terminators); i++)
    {
        end = strchr(str, terminators[i]);
    }
    return end;
}

static int parse_value(HARGPARSE_VALUE* ref, tArgsparseValuetype type, const char* value)
{
    int ret = 0;
    if (type == ARGSPARSE_TYPE_NONE)
        return ret;

    if (*ref == NULL)
        *ref = calloc(1, sizeof(t_argparse_value));
    switch (type)
    {
        case ARGSPARSE_TYPE_DOUBLE:
            (*ref)->doublevalue = value ? atof(value) : -1.0;
        break;
        case ARGSPARSE_TYPE_INT:
            (*ref)->intvalue = value ? atoi(value) : -1;
        break;
        case ARGSPARSE_TYPE_STRING:
        {
            const char* end = FindEndTerminator(value);
            if (end)
            {
                int len = end - value;
                if (len > 0 && len < ARGSPARSE_VALUE_STRING_SIZE)
                {
                    strncpy((*ref)->stringvalue, value, len);
                    (*ref)->stringvalue[len] = 0;
                }
                else
                    ret = -1;
            }
            else
                ret = -1;
        }
        break;
        default:
            ret = -1;
        break;
    }
    return ret;
}

int set_short_option(char c, HARGPARSE_HANDLE handle, HARGPARSE_ARG arg)
{
    int ret = -1;
    char* used = handle->shortopts;
    // iterate used until terminating 0
    while (*used)
    {
        // used - break the loop and move to next longname character
        if (*used == c)
        {
            break;
        }
        used++;
    }
    // unused - set value and break
    if (*used == 0)
    {
        *used = c;
        *(used + 1) = (arg->type == ARGSPARSE_TYPE_NONE) ? 0 : ':';
        arg->name_short = c;
        ret = 0;
    }
    return ret;
}

void generate_short_name(HARGPARSE_HANDLE handle, HARGPARSE_ARG arg)
{
    char* longname = arg->name;
    char c;
    // iterate characters of longname as long as necessary
    while(c = *longname)
    {
        if (set_short_option(c, handle, arg))
        {
            longname++;
            continue;
        }
        break;
    }
    // none of the longname characters accepted
    if (!c)
    {
        // iterate characters from 'a' until the short option is set
        c = 'a';
        while (1)
        {
            if (set_short_option(c, handle, arg))
            {
                c++;
                continue;
            }
            break;
        }
    }
}

/// @brief Add argument moves argument ownership to handle
/// @param handle Handle to allocated arguments structure
/// @param argument Handle to allocated argument
int argsparse_put_argument(HARGPARSE_HANDLE handle, HARGPARSE_ARG* href)
{
    if (handle->count >= ARGSPARSE_MAX_ARGUMENTS)
    {
        // didn't take ownership
        return -1;
    }
    handle->count++;
    HARGPARSE_ARG arg = *href;
    HARGPARSE_ARG_LINKED linked = calloc(1, sizeof(t_argparse_argument_linked));
    linked->argument = arg;
    generate_short_name(handle, arg);
    if (handle->arguments)
    {
        HARGPARSE_ARG_LINKED next = handle->arguments;
        while (next->next)
        {
            next = next->next;
        }
        next->next = linked;
    }
    else
        handle->arguments = linked;
    // take ownership
    *href = NULL;
    return 0;
}

void argsparse_add_int(HARGPARSE_HANDLE handle, const char* option, const char* description, int value)
{

}

void argsparse_add_double(HARGPARSE_HANDLE handle, const char* option, const char* description, double value)
{

}

void AddArgumentTypeOnOff(HARGPARSE_HANDLE handle, const char* option, const char* description, const char* value)
{

}

void argsparse_add_str(HARGPARSE_HANDLE handle, const char* option, const char* description, const char* value)
{

}

int argsparse_add_flag(HARGPARSE_HANDLE handle, const char* name, const char* description)
{
    HARGPARSE_ARG p = argsparse_argument_allocate(name, description);
    p->type = ARGSPARSE_TYPE_NONE;
    int err = argsparse_put_argument(handle, &p);
    if (err)
        free (p);
    return err;
}

int argsparse_parse_args(HARGPARSE_HANDLE handle, char* const *argv, int argc)
{
    int count = 0;
    struct option *long_options = calloc(count, sizeof(struct option));
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
    argsparse_argument_action(handle, do_option_long, (void*)long_options);
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

        HARGPARSE_ARG arg = argsparse_argument_by_short_name(handle, c);
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
    return count;
}

void argsparse_usage(HARGPARSE_HANDLE handle)
{

}

static HARGPARSE_ARG_LINKED delete_argument(HARGPARSE_ARG_LINKED del)
{
    HARGPARSE_ARG_LINKED ret = del != NULL ? del->next : NULL;
    del->next = NULL;
    if (del != NULL)
    {
        HARGPARSE_ARG arg = del->argument; 
        del->argument = NULL;
        if (arg->value)
            free (arg->value);
        arg->value = NULL;
        free (arg->name);
        arg->name = NULL;
        free (arg->description);
        arg->description = NULL;
        free(arg);
        free(del);
    }
    return ret;
}