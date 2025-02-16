#ifndef INTERNAL_FUNCS_H
#define INTERNAL_FUNCS_H

#include "internal_types.h"

HARGPARSE_ARG_LINKED free_linked_argument(HARGPARSE_ARG_LINKED arg);
ARG_ARGUMENT_HANDLE create_argument(const char* name, const char* desc);
void free_argument(ARG_ARGUMENT_HANDLE* handle);

void copy_to_argument_string(char* dest, const char* source);
int parse_value(ARG_VALUE* ref, ARG_TYPE type, void* value);
int set_short_option(char c, ARG_DATA_HANDLE handle, ARG_ARGUMENT_HANDLE arg);
void generate_short_name(ARG_DATA_HANDLE handle, ARG_ARGUMENT_HANDLE arg);

#endif