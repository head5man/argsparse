#ifndef ITERATE_H
#define ITERATE_H

#include "internal_types.h"

static ARG_ARGUMENT_HANDLE iterate_arguments_return_on_zero(ARG_DATA_HANDLE handle, int(*predicate)(int, ARG_ARGUMENT_HANDLE, void*), void* data);

static int action_show_argument_value(int idx, ARG_ARGUMENT_HANDLE arg, void* data);
static int action_show_argument_usage(int idx, ARG_ARGUMENT_HANDLE arg, void* data);
static int action_long_option_width(int idx, ARG_ARGUMENT_HANDLE arg, void* data);
static int action_mark_parsed_flags(int idx, ARG_ARGUMENT_HANDLE arg, void* data);
static int action_do_option_long(int idx, ARG_ARGUMENT_HANDLE arg, void* data);
static int action_count(int idx, ARG_ARGUMENT_HANDLE arg, void* data);

static int predicate_compare_name(int idx, ARG_ARGUMENT_HANDLE arg, void* data);
static int predicate_compare_short_name(int idx, ARG_ARGUMENT_HANDLE arg, void* data);

#endif