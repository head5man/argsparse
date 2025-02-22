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

#ifndef ARGSPARSE_MAX_STRING_SIZE
#   define ARGSPARSE_MAX_STRING_SIZE 80
#endif

#ifndef ARGSPARSE_MAX_ARGS
#   define ARGSPARSE_MAX_ARGS 40
#endif

typedef enum _argsparse_errors {
    ERROR_NONE = 0,
    ERROR_UNKNOWN = -1,
    ERROR_MAX_ARGS = -2,
    ERROR_EXISTS = -3,
} e_argsparse_errors;

typedef enum _argsparse_type {
    /// @brief illegal value
    ARGSPARSE_TYPE_NONE = -1,
    ARGSPARSE_TYPE_STRING,
    ARGSPARSE_TYPE_INT,
    ARGSPARSE_TYPE_DOUBLE,
    /// @brief value of flag can be tested against zero
    ARGSPARSE_TYPE_FLAG,
    /// @brief count of legal types
    ARGSPARSE_TYPE_CNT
} argsparse_type_e;

typedef union _argparse_value
{
    char stringvalue[ARGSPARSE_MAX_STRING_SIZE];
    int* flagptr;
    int intvalue;
    double doublevalue;
} argsparse_value_t;

typedef union _argparse_value ARG_VALUE;

typedef struct _argparse_argument
{
    argsparse_type_e type;
    int parsed;
    int name_short;
    char name[ARGSPARSE_MAX_STRING_SIZE];
    char description[ARGSPARSE_MAX_STRING_SIZE];
    union {
        int flagvalue;
        ARG_VALUE initvalue;
    } flag_init;
    ARG_VALUE value;
} argsparse_argument_t;

typedef struct _argparse_argument* ARG_ARGUMENT_HANDLE;
typedef struct _argparse_data* ARG_DATA_HANDLE;
typedef enum _argsparse_type ARG_TYPE;
typedef enum _argsparse_errors ARG_ERROR;

/// @brief Create arguments structure 
/// @param title 
/// @return handle
ARG_DATA_HANDLE argsparse_create(const char* title);

/// @brief Allocate arguments structure 
/// @param title 
/// @return handle
void argsparse_free(ARG_DATA_HANDLE handle);

ARG_ARGUMENT_HANDLE argsparse_create_argument_with_value(ARG_TYPE type, const char* option, const char* desc, ARG_VALUE* value);

/// @brief Add argument moves argument ownership to handle
/// @param handle Handle to allocated arguments structure
/// @param argument Handle to allocated argument
/// @return 0 - success, !0 - arg not consumed
ARG_ERROR argsparse_put_argument(ARG_DATA_HANDLE handle, ARG_ARGUMENT_HANDLE* argument);

/// @brief Add int argument
/// @param handle Handle to allocated arguments structure
/// @param option Option cmdline flag
/// @param description Description of argument
/// @param value Default value
/// @return
/// ERROR_NONE(0) - success
///
/// ERROR_MAX_ARGS(1) - ARGSPARSE_MAX_ARGS reached, not added
ARG_ERROR argsparse_add_int(ARG_DATA_HANDLE handle, const char* option, const char* description, int value);

/// @brief Add DOUBLE argument
/// @param handle Handle to allocated arguments structure
/// @param option Option cmdline flag
/// @param description Description of argument
/// @param value Default value
/// @return
/// ERROR_NONE(0) - success
///
/// ERROR_MAX_ARGS(1) - ARGSPARSE_MAX_ARGS reached, not added
ARG_ERROR argsparse_add_double(ARG_DATA_HANDLE handle, const char* option, const char* description, double value);

/// @brief Add string argument
/// @param handle Handle to allocated arguments structure
/// @param option Option cmdline flag
/// @param description Description of argument
/// @param value Default value
/// @return
/// ERROR_NONE(0) - success
///
/// ERROR_MAX_ARGS(1) - ARGSPARSE_MAX_ARGS reached, not added
ARG_ERROR argsparse_add_cstr(ARG_DATA_HANDLE handle, const char* option, const char* description, const char* value);

/// @brief Add argument flag only
/// @param handle allocated arguments structure handle
/// @param name argument name
/// @param desc argument description
/// @param value Value to set when option present
/// @param ptr_to_value pointer to value or null
/// @return
/// ERROR_NONE(0) - success
/// ERROR_EXISTS - argument with same name already exists
/// ERROR_MAX_ARGS(1) - ARGSPARSE_MAX_ARGS reached, not added

/// @return
/// ERROR_NONE(0) - success
///
/// ERROR_MAX_ARGS(1) - ARGSPARSE_MAX_ARGS reached, not added
ARG_ERROR argsparse_add_flag(ARG_DATA_HANDLE handle, const char* option, const char* description, int value, int* ptr_to_value);

/// @brief Parse cmdline argument against added arguments 
/// @param handle Handle to allocated arguments structure
/// @param argsv 
/// @param argc 
int argsparse_parse_args(ARG_DATA_HANDLE handle, char* const* argv, int argc);

/// @brief Prints all arguments in usage message
/// @param handle 
void argsparse_usage(ARG_DATA_HANDLE handle, const char* const executable);

const char* argsparse_get_title(ARG_DATA_HANDLE handle);
char* argsparse_get_shortopts(ARG_DATA_HANDLE handle);

ARG_ARGUMENT_HANDLE argsparse_argument_by_name(ARG_DATA_HANDLE handle, const char* name);
ARG_ARGUMENT_HANDLE argsparse_argument_by_short_name(ARG_DATA_HANDLE handle, int shortname);
int argsparse_argument_count(ARG_DATA_HANDLE handle);

#if defined( __cplusplus )
}
#endif