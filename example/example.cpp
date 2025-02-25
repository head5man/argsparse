#include "argsparse.h"

#include <stdio.h>

int gFlag = 0;

int err_print(int err, const char *message);

void initialize_arguments(ARG_DATA_HANDLE arguments);

void arg_print(ARG_ARGUMENT_HANDLE arg);

int main(int argc, char **argv)
{
    ARG_DATA_HANDLE arguments = argsparse_create("argsparse-example");
    try
    {
        initialize_arguments(arguments);
        int parsed = argsparse_parse_args(arguments, argv, argc);
        printf("shortopts %s - %d arguments parsed\n", argsparse_get_shortopts(arguments), parsed);
        argsparse_show_arguments(arguments);
    }
    catch (...)
    {
        printf("exit due to exception");
    }
    argsparse_free(arguments);
}

void initialize_arguments(ARG_DATA_HANDLE arguments)
{
    ARG_ERROR err = ERROR_AP_NONE;
    err = argsparse_add_help(arguments);
    err_print(err, "help not added");
    err = argsparse_add_int(arguments, "integer", "This is an integer value", 0);
    err_print(err, "integer not added");

    err = argsparse_add_double(arguments, "double", "This is a double value", 0.0);
    err_print(err, "double not added");

    err = argsparse_add_cstr(arguments, "string", "This is a string value", "");
    err_print(err, "string not added");

    err = argsparse_add_flag(arguments, "flag", "This is a flag value", 123, &gFlag);
    err_print(err, "flag not added");
}

int err_print(int err, const char* message)
{
    if (err != ERROR_AP_NONE)
    {
        printf("ERR(%d): %s\n", err, message);
        throw err;
    }
    return err;
}

void arg_print(ARG_ARGUMENT_HANDLE arg)
{
    if (0 == err_print(arg == NULL, "Got null for argument"))
    {
        const char* int_fmt = "long: %s short: '%c' value: %d\n";
        const char* dbl_fmt = "long: %s short: '%c' value: %f\n";
        const char* str_fmt = "long: %s short: '%c' value: %s\n";
        const char* flg_fmt = "long: %s short: '%c' value: %d\n";
        switch (arg->type)
        {
            case ARGSPARSE_TYPE_INT:
                printf(int_fmt, arg->name, arg->name_short, arg->value.intvalue);
                break;
            case ARGSPARSE_TYPE_DOUBLE:
                printf(dbl_fmt, arg->name, arg->name_short, arg->value.doublevalue);
                break;
            case ARGSPARSE_TYPE_STRING:
                printf(str_fmt, arg->name, arg->name_short, arg->value.stringvalue);
                break;
            case ARGSPARSE_TYPE_FLAG:
                printf(flg_fmt, arg->name, arg->name_short, *arg->value.flagptr);
                break;
            default:
                printf("Unsupported type");
                break;
        }
    }
}