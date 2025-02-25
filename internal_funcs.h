#ifndef INTERNAL_FUNCS_H
#define INTERNAL_FUNCS_H

#include "internal_types.h"

#include <stddef.h>

static ARG_ARGUMENT_HANDLE create_argument(ARG_TYPE type, const char* name, const char* description, const ARG_VALUE* value);
static void free_argument(ARG_ARGUMENT_HANDLE* handle);
static HARGPARSE_ARG_LINKED free_linked_argument(HARGPARSE_ARG_LINKED arg);

/// @brief Add argument moves argument ownership to handle
/// @param handle Handle to allocated arguments structure
/// @param argument Handle to allocated argument
/// @return
/// ERROR_AP_NONE(0) - success
///
/// ERROR_AP_EXISTS - argument with same name already exists
///
/// ERROR_AP_MAX_ARGS(1) - ARGSPARSE_MAX_ARGS reached, not added
static ARG_ERROR put_argument(ARG_DATA_HANDLE handle, ARG_ARGUMENT_HANDLE* argument);

void copy_to_argument_string(char* dest, const char* source);
int parse_value(ARG_VALUE* ref, ARG_TYPE type, const char* value);
int set_short_option(char c, ARG_DATA_HANDLE handle, ARG_ARGUMENT_HANDLE arg);
void generate_short_name(ARG_DATA_HANDLE handle, ARG_ARGUMENT_HANDLE arg);
const char* get_argument_type_string(ARG_TYPE type);
const char* get_argument_value_string(ARG_ARGUMENT_HANDLE arg, char* buffer, size_t buflen);
#endif