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
    ERROR_AP_NONE = 0,
    ERROR_AP_UNKNOWN = -1,
    ERROR_AP_MAX_ARGS = -2,
    ERROR_AP_EXISTS = -3,
    ERROR_AP_MEMORY = -4,
} e_argsparse_errors;

typedef enum _argsparse_type {
    /// @brief illegal/reserved value
    ARGSPARSE_TYPE_NONE = -1,
    ARGSPARSE_TYPE_STRING,
    ARGSPARSE_TYPE_INT,
    ARGSPARSE_TYPE_DOUBLE,
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
/// @param handle 
void argsparse_free(ARG_DATA_HANDLE handle);

/// @brief Adds argument using the structured format
/// @param handle
/// @param name
/// @param description
/// @param type
/// @param value
/// @return
/// ERROR_AP_NONE(0) - success
///
/// ERROR_AP_EXISTS - argument with same name already exists
///
/// ERROR_AP_MAX_ARGS(1) - ARGSPARSE_MAX_ARGS reached, not added
/// @note Smells like internal, but having invested
/// a quite a lot to testing it decided to drag it along.
ARG_ERROR argsparse_add(ARG_DATA_HANDLE handle, const char* name, const char* description, ARG_TYPE type, const ARG_VALUE* value);

/// @brief Add help option showing usage with exit
/// @param handle allocated arguments structure handle
/// @return
/// ERROR_AP_NONE(0) - success
///
/// ERROR_AP_EXISTS - argument with same name already exists
///
/// ERROR_AP_MAX_ARGS(1) - ARGSPARSE_MAX_ARGS reached, not added
ARG_ERROR argsparse_add_help(ARG_DATA_HANDLE handle);

/// @brief Add int argument
/// @param handle allocated arguments structure handle
/// @param name argument name
/// @param desc argument description
/// @param value argument value (default)
/// @return
/// ERROR_AP_NONE(0) - success
///
/// ERROR_AP_EXISTS - argument with same name already exists
///
/// ERROR_AP_MAX_ARGS(1) - ARGSPARSE_MAX_ARGS reached, not added
ARG_ERROR argsparse_add_int(ARG_DATA_HANDLE handle, const char* name, const char* desc, int value);

/// @brief Add DOUBLE argument
/// @param handle allocated arguments structure handle
/// @param name argument name
/// @param description argument description
/// @param value argument value (default)
/// @return
/// ERROR_AP_NONE(0) - success
///
/// ERROR_AP_EXISTS - argument with same name already exists
///
/// ERROR_AP_MAX_ARGS(1) - ARGSPARSE_MAX_ARGS reached, not added
ARG_ERROR argsparse_add_double(ARG_DATA_HANDLE handle, const char* name, const char* description, double value);

/// @brief Add string argument
/// @param handle allocated arguments structure handle
/// @param name argument name
/// @param description argument description
/// @param value argument value (default)
/// @return
/// ERROR_AP_NONE(0) - success
///
/// ERROR_AP_EXISTS - argument with same name already exists
///
/// ERROR_AP_MAX_ARGS(1) - ARGSPARSE_MAX_ARGS reached, not added
ARG_ERROR argsparse_add_cstr(ARG_DATA_HANDLE handle, const char* name, const char* description, const char* value);

/// @brief Add argument flag only
/// @param handle allocated arguments structure handle
/// @param name argument name
/// @param description argument description
/// @param value Value to set when option present
/// @param ptr_to_value pointer to value or null
/// @return
/// ERROR_AP_NONE(0) - success
///
/// ERROR_AP_EXISTS - argument with same name already exists
///
/// ERROR_AP_MAX_ARGS(1) - ARGSPARSE_MAX_ARGS reached, not added
ARG_ERROR argsparse_add_flag(ARG_DATA_HANDLE handle, const char* name, const char* description, int value, int* ptr_to_value);

/// @brief Parse cmdline argument against added arguments 
/// @param handle Handle to allocated arguments structure
/// @param argsv
/// @param argc
int argsparse_parse_args(ARG_DATA_HANDLE handle, char* const* argv, int argc);

/// @brief Prints usage message
/// @param handle
void argsparse_show_usage(ARG_DATA_HANDLE handle, const char* const executable);

/// @brief Prints argument values
/// @param handle
void argsparse_show_arguments(ARG_DATA_HANDLE handle);

/// @brief Get title
/// @param handle
/// @return string
const char* argsparse_get_title(ARG_DATA_HANDLE handle);

/// @brief Get short options
/// @param handle
/// @return
char* argsparse_get_shortopts(ARG_DATA_HANDLE handle);

/// @brief Get argument by name
/// @param handle
/// @param name
/// @return handle to argument
ARG_ARGUMENT_HANDLE argsparse_argument_by_name(ARG_DATA_HANDLE handle, const char* name);

/// @brief Get argument by short name
/// @param handle
/// @param name
/// @return handle to argument
ARG_ARGUMENT_HANDLE argsparse_argument_by_short_name(ARG_DATA_HANDLE handle, int shortname);

/// @brief Get argument count
/// @param handle
/// @return count
int argsparse_argument_count(ARG_DATA_HANDLE handle);

#if defined( __cplusplus )
}
#endif