
#include "internal_funcs.h"

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

const char* get_argument_type_string(ARG_TYPE type)
{
    switch (type)
    {
        case ARGSPARSE_TYPE_DOUBLE:
            return "dbl";
        case ARGSPARSE_TYPE_NONE:
            return "nul";
        case ARGSPARSE_TYPE_FLAG:
            return "flg";
        case ARGSPARSE_TYPE_INT:
            return "int";
        case ARGSPARSE_TYPE_STRING:
            return "str";
        default:
            return "wtf";
    }
}

const char* get_argument_value_string(ARG_ARGUMENT_HANDLE arg, char* buffer, size_t buflen)
{
    switch (arg->type)
    {
        case ARGSPARSE_TYPE_FLAG:
            // flag
            snprintf(buffer, buflen, "%d:%d", *(arg->value.flagptr), arg->flag_init.flagvalue);
            break;
        case ARGSPARSE_TYPE_INT:
            // integer
            snprintf(buffer, buflen, "%d", arg->value.intvalue);
            break;
        case ARGSPARSE_TYPE_DOUBLE:
            // double
            snprintf(buffer, buflen, "%f", arg->value.doublevalue);
            break;
        case ARGSPARSE_TYPE_STRING:
            snprintf(buffer, buflen, "%s", arg->value.stringvalue);
        default:
            break;
    }
    return buffer;
}

void copy_to_argument_string(char* dest, const char* source)
{
    size_t len = strlen(source);
    len = len < ARGSPARSE_MAX_STRING_SIZE ? len : ARGSPARSE_MAX_STRING_SIZE - 1;
    strncpy(dest, source, len);
    dest[len] = '\0';
}

const char* find_string_end(const char* str)
{
    char terminators[] = {'\0'};
    const char* end = NULL;
    if (str != NULL)
    {
        for (int i = 0; str && end == NULL && i < sizeof(terminators); i++)
        {
            end = strchr(str, terminators[i]);
        }
    }
    return end;
}

int parse_value(ARG_VALUE* ref, ARG_TYPE type, const char* str_value)
{
    int ret = 0;

    if (type == ARGSPARSE_TYPE_NONE)
        return -1;
    
    switch (type)
    {
        case ARGSPARSE_TYPE_FLAG:
            // getopt_long already set the option.val to option.flag
            break;
        case ARGSPARSE_TYPE_DOUBLE:
            ref->doublevalue = str_value ? atof(str_value) : -1.0;
        break;
        case ARGSPARSE_TYPE_INT:
            ref->intvalue = str_value ? atoi(str_value) : -1;
        break;
        case ARGSPARSE_TYPE_STRING:
        {
            const char* end = find_string_end(str_value);
            if (end)
            {
                ptrdiff_t len = end - str_value;
                if (len > 0 && len < ARGSPARSE_MAX_STRING_SIZE)
                {
                    strncpy(ref->stringvalue, str_value, len);
                    ref->stringvalue[len] = 0;
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

int set_short_option(char c, ARG_DATA_HANDLE handle, ARG_ARGUMENT_HANDLE arg)
{
    int ret = ERROR_EXISTS;
    char* opt = handle->shortopts;
    // iterate used until terminating 0
    while (*opt)
    {
        if (*opt != ':' && *opt == c)
        {
            break;
        }
        opt++;
    }
    // unused - set value and break
    if (*opt == 0)
    {
        *opt = c;
        if ((arg->type == ARGSPARSE_TYPE_FLAG) || (arg->type == ARGSPARSE_TYPE_NONE))
        {
            *(opt + 1) = '\0';
        }
        else
        {
            *(opt + 1) = ':';
            *(opt + 2) = '\0';
        }
        arg->name_short = c;
        ret = ERROR_NONE;
    }

    return ret;
}

char iterate_set_of_chars_for_short(const char* sopts, const char* charset)
{
    if (charset != NULL && sopts != NULL)
    {
        size_t sopts_len = strlen(sopts);
        while (*charset)
        {
            char c = *charset;
            char* used = sopts_len <= 0 ? NULL : strchr(sopts, c);
            if (used == NULL)
            {
                // found unused char
                return c;
            }
            charset++;
        }
    }
    return 0;
}

void generate_short_name(ARG_DATA_HANDLE handle, ARG_ARGUMENT_HANDLE arg)
{
    if (arg->type == ARGSPARSE_TYPE_FLAG)
        return;

    char* longname = arg->name;
    char shortopt = iterate_set_of_chars_for_short(handle->shortopts, longname);
    if (shortopt == '\0')
    {
        shortopt = iterate_set_of_chars_for_short(handle->shortopts, "abcdefghiklmnopqrstuvwxyz");
    }

    set_short_option(shortopt, handle, arg);
}