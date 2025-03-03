#ifndef INTERNAL_TYPES_H
#define INTERNAL_TYPES_H

#include "argsparse.h"

typedef struct _argparse_argument_linked
{
    ARG_ARGUMENT_HANDLE argument;
    void* next;
} t_argparse_argument_linked;

typedef struct _argparse_argument_linked* HARGPARSE_ARG_LINKED;
typedef struct _argparse_data
{
    char shortopts[ARGSPARSE_MAX_ARGS * 2];
    int count;
    HARGPARSE_ARG_LINKED arguments;
    const char* title;
} argument_data_t;

#endif