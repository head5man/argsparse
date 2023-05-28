/**
 * @file argsparse.h
 * @author Tuomas Lahtinen (tuomas123lahtinen@gmail.com)
 * @brief Generic command line arguments definition utility
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#if defined( __cplusplus )
extern "C"
{
#endif

#ifndef ARGSPARSE_VALUE_UNION_SIZE
#   define ARGSPARSE_VALUE_STRING_SIZE 100
#endif

#ifndef ARGSPARSE_MAX_ARGUMENTS
#   define ARGSPARSE_MAX_ARGUMENTS 50
#endif

typedef enum {
    ARGSPARSE_TYPE_STRING,
    ARGSPARSE_TYPE_INT,
    ARGSPARSE_TYPE_DOUBLE,
    ARGSPARSE_TYPE_NONE,
    ARGSPARSE_TYPE_CNT
} tArgsparseValuetype;

typedef union _argparse_value
{
    char stringvalue[ARGSPARSE_VALUE_STRING_SIZE + 1];
    int intvalue;
    double doublevalue;
} t_argparse_value;

typedef union _argparse_value* HARGPARSE_VALUE;

typedef struct _argparse_argument
{
    int parsed;
    tArgsparseValuetype type;
    char* name;
    int name_short;
    char* description;
    HARGPARSE_VALUE value;
} t_argparse_argument;

typedef struct _argparse_argument* HARGPARSE_ARG;
typedef struct _argparse_struct* HARGPARSE_HANDLE;

/// @brief Create arguments structure 
/// @param title 
/// @return handle
HARGPARSE_HANDLE argsparse_create(const char* title);
/// @brief Allocate arguments structure 
/// @param title 
/// @return handle
void argsparse_delete(HARGPARSE_HANDLE handle);

HARGPARSE_ARG argsparse_create_argument(tArgsparseValuetype type, const char* option, const char* desc, const char* value);

/// @brief Add argument moves argument ownership to handle
/// @param handle Handle to allocated arguments structure
/// @param argument Handle to allocated argument
/// @return 0 - success, !0 - arg not consumed
int argsparse_put_argument(HARGPARSE_HANDLE handle, HARGPARSE_ARG* argument);

/// @brief Add int argument
/// @param handle Handle to allocated arguments structure
/// @param option Option cmdline flag
/// @param description Description of argument
/// @param value Default value
void argsparse_add_int(HARGPARSE_HANDLE handle, const char* option, const char* description, int value);
/// @brief Add DOUBLE argument
/// @param handle Handle to allocated arguments structure
/// @param option Option cmdline flag
/// @param description Description of argument
/// @param value Default value
void argsparse_add_double(HARGPARSE_HANDLE handle, const char* option, const char* description, double value);
/// @brief Add string argument
/// @param handle Handle to allocated arguments structure
/// @param option Option cmdline flag
/// @param description Description of argument
/// @param value Default value
void argsparse_add_str(HARGPARSE_HANDLE handle, const char* option, const char* description, const char* value);
/// @brief Add argument flag only
/// @param handle Handle to allocated arguments structure
/// @param option Option cmdline flag
/// @param description Description of argument
int argsparse_add_flag(HARGPARSE_HANDLE handle, const char* option, const char* description);
/// @brief Parse cmdline argument against added arguments 
/// @param handle Handle to allocated arguments structure
/// @param argsv 
/// @param argc 
int argsparse_parse_args(HARGPARSE_HANDLE handle, char* const *argv, int argc);
/// @brief Prints all arguments in usage message
/// @param handle 
void argsparse_usage(HARGPARSE_HANDLE handle);

const char* argsparse_get_title(HARGPARSE_HANDLE handle);
char* argsparse_get_shortopts(HARGPARSE_HANDLE handle);
HARGPARSE_ARG argsparse_argument_get(HARGPARSE_HANDLE handle, int(*cmp)(HARGPARSE_ARG, void*), void* cmpdata);
HARGPARSE_ARG argsparse_argument_by_name(HARGPARSE_HANDLE handle, const char* name);
HARGPARSE_ARG argsparse_argument_by_short_name(HARGPARSE_HANDLE handle, int shortname);
int argsparse_argument_count(HARGPARSE_HANDLE handle);

#if defined( __cplusplus )
}
#endif