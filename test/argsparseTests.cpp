/**
 * @file argsparseTests.cpp
 * @author Tuomas Lahtinen (tuomas123lahtinen@gmail.com)
 * @brief 
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "argsparse.h"
#include "tokenize.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gtest/gtest-matchers.h"
#include <sstream>
#include <ostream>
#include <memory>

namespace argsparse::testing
{
#define TEST_FIXTURE argsparse_test
using ::testing::NotNull;
using ::testing::IsNull;

#define BUFFER_SIZE 100
#define ARGV_SIZE 10

ARG_DATA_HANDLE gHandle = nullptr;
char gBuffer[BUFFER_SIZE] = { 0, };

char* gArgv[ARGV_SIZE] = { nullptr, };
int gArgc = 0;

class TEST_FIXTURE:public ::testing::TestWithParam<ARG_TYPE>
{
  protected:
    void SetUp() override
    {
        memset(gBuffer, 0, sizeof(gBuffer));
        memset(gArgv, 0, sizeof(gArgv));
        gArgc = 0;
    }

    void TearDown() override
    {
       argsparse_free(gHandle);
       gHandle = nullptr;
    }
};

void print_arguments(char* const* argv, int argc)
{
    for (int i = 0; i < argc; i++)
    {
        std::cerr << "argv["<< i <<"]: "<< argv[i] <<"\n";
    }
}

TEST_F(TEST_FIXTURE, ShouldAllocateHandle)
{
    gHandle = argsparse_create(NULL);
    ASSERT_THAT(gHandle, NotNull());
}

TEST_F(TEST_FIXTURE, HandleShouldGetTitle)
{
    const char* title = "Testing version 1.0 - Arguments";
    gHandle = argsparse_create(title);
    ASSERT_STREQ(title, argsparse_get_title(gHandle));
}

TEST_F(TEST_FIXTURE, ShouldAppendShortOptions)
{
    gHandle = argsparse_create(NULL);
    std::cerr << "1" << std::endl; 
    char* shortopts = argsparse_get_shortopts(gHandle);
    std::cerr << "2" << std::endl;
    ASSERT_EQ(0, *shortopts);
    std::cerr << "3" << std::endl;
    ARG_ARGUMENT_HANDLE harg = argsparse_create_argument_with_value(ARG_TYPE::ARGSPARSE_TYPE_INT, "flag", "description", "1234");
    std::cerr << "4" << std::endl;
    argsparse_put_argument(gHandle, &harg);
    std::cerr << "5" << std::endl;
    ASSERT_STREQ("f:", shortopts);
}

TEST_F(TEST_FIXTURE, ShouldAddManyArguments)
{
    ARG_ERROR err = ERROR_NONE;
    gHandle = argsparse_create(NULL);
    char flag[] = { 'f', 'l', 'a', 'g', '?','?','?', 0 };
    const char* fmt = "flag%d";
    for (int i = 0; i <= ARGSPARSE_MAX_ARGS; i++)
    {
        sprintf(flag, fmt, i);
        ARG_ARGUMENT_HANDLE h = argsparse_create_argument_with_value(
            (ARG_TYPE)(i % ARGSPARSE_TYPE_CNT),
            flag, 
            "This is one of the many flags created", 
            NULL);
        err = argsparse_put_argument(gHandle, &h);
        if (err != ERROR_NONE)
            break;
    }
    ASSERT_EQ(ARG_ERROR::ERROR_MAX_ARGS, err);
    ASSERT_EQ(ARGSPARSE_MAX_ARGS, argsparse_argument_count(gHandle));
}

TEST_F(TEST_FIXTURE, ShouldParseOptionLongFlag)
{
    int argc = 0;
    sprintf(gBuffer, "program --flag");
    tokenise_to_argc_argv(gBuffer, &argc, gArgv, ARGV_SIZE, print_arguments);

    gHandle = argsparse_create(NULL);
    argsparse_add_flag(gHandle, "flag", "This is a flag", 0);
    
    ASSERT_EQ(1, argsparse_parse_args(gHandle, gArgv, argc));

    ARG_ARGUMENT_HANDLE arg = argsparse_argument_by_name(gHandle, "flag");
    ASSERT_EQ(1, arg->parsed);
    ASSERT_NE(0, arg->value.flagvalue);
}

TEST_F(TEST_FIXTURE, ShouldParseOptionLongInt)
{
    int argc = 0;
    sprintf(gBuffer, "program --integer 4321");
    tokenise_to_argc_argv(gBuffer, &argc, gArgv, ARGV_SIZE, print_arguments);

    gHandle = argsparse_create(NULL);
    argsparse_add_int(gHandle, "integer", "This is an integer", 1234);
    ARG_ARGUMENT_HANDLE arg = argsparse_argument_by_name(gHandle, "integer");
    ASSERT_THAT(arg, NotNull());
    ASSERT_EQ(1234, arg->value.intvalue);
    ASSERT_EQ(1, argsparse_parse_args(gHandle, gArgv, argc));
    ASSERT_EQ(1, arg->parsed);
    ASSERT_EQ(4321, arg->value.intvalue);
}

TEST_F(TEST_FIXTURE, ShouldParseOptionLongInt2)
{
    int argc = 0;
    sprintf(gBuffer, "program --integer=4321");
    tokenise_to_argc_argv(gBuffer, &argc, gArgv, ARGV_SIZE, print_arguments);

    gHandle = argsparse_create(NULL);
    argsparse_add_int(gHandle, "integer", "This is an integer", 1234);
    ARG_ARGUMENT_HANDLE arg = argsparse_argument_by_name(gHandle, "integer");
    ASSERT_THAT(arg, NotNull());
    ASSERT_EQ(1234, arg->value.intvalue);
    ASSERT_EQ(1, argsparse_parse_args(gHandle, gArgv, argc));
    ASSERT_EQ(1, arg->parsed);
    ASSERT_EQ(4321, arg->value.intvalue);
}

TEST_F(TEST_FIXTURE, ShouldParseOptionShortInt)
{
    int argc = 0;
    sprintf(gBuffer, "program -i 4321");
    tokenise_to_argc_argv(gBuffer, &argc, gArgv, ARGV_SIZE, print_arguments);

    gHandle = argsparse_create(NULL);
    argsparse_add_int(gHandle, "integer", "This is an integer", 1234);
    ARG_ARGUMENT_HANDLE arg = argsparse_argument_by_name(gHandle, "integer");
    ASSERT_THAT(arg, NotNull());
    ASSERT_EQ(1234, arg->value.intvalue);
    ASSERT_EQ(1, argsparse_parse_args(gHandle, gArgv, argc));
    ASSERT_EQ(1, arg->parsed);
    ASSERT_EQ(4321, arg->value.intvalue);
}

TEST_F(TEST_FIXTURE, ShouldParseOptionShortDouble)
{
    int argc = 0;
    sprintf(gBuffer, "program -d 4321.1234");
    tokenise_to_argc_argv(gBuffer, &argc, gArgv, ARGV_SIZE, print_arguments);

    gHandle = argsparse_create(NULL);
    argsparse_add_double(gHandle, "double", "This is a double", 1234.4321);
    ARG_ARGUMENT_HANDLE arg = argsparse_argument_by_name(gHandle, "double");
    ASSERT_THAT(arg, NotNull());
    ASSERT_EQ(1234.4321, arg->value.doublevalue);
    ASSERT_EQ(1, argsparse_parse_args(gHandle, gArgv, argc));
    ASSERT_EQ(1, arg->parsed);
    ASSERT_EQ(4321.1234, arg->value.doublevalue);
}

TEST_F(TEST_FIXTURE, ShouldParseOptionLongDouble1)
{
    int argc = 0;
    sprintf(gBuffer, "program --double 4321.1234");
    tokenise_to_argc_argv(gBuffer, &argc, gArgv, ARGV_SIZE, print_arguments);

    gHandle = argsparse_create(NULL);
    argsparse_add_double(gHandle, "double", "This is a double", 1234.4321);
    ARG_ARGUMENT_HANDLE arg = argsparse_argument_by_name(gHandle, "double");
    ASSERT_THAT(arg, NotNull());
    ASSERT_EQ(1234.4321, arg->value.doublevalue);
    ASSERT_EQ(1, argsparse_parse_args(gHandle, gArgv, argc));
    ASSERT_EQ(1, arg->parsed);
    ASSERT_EQ(4321.1234, arg->value.doublevalue);
}

TEST_F(TEST_FIXTURE, ShouldParseOptionLongDouble2)
{
    int argc = 0;
    sprintf(gBuffer, "program --double=4321.1234");
    tokenise_to_argc_argv(gBuffer, &argc, gArgv, ARGV_SIZE, print_arguments);

    gHandle = argsparse_create(NULL);
    argsparse_add_double(gHandle, "double", "This is a double", 1234.4321);
    ARG_ARGUMENT_HANDLE arg = argsparse_argument_by_name(gHandle, "double");
    ASSERT_THAT(arg, NotNull());
    ASSERT_EQ(1234.4321, arg->value.doublevalue);
    ASSERT_EQ(1, argsparse_parse_args(gHandle, gArgv, argc));
    ASSERT_EQ(1, arg->parsed);
    ASSERT_EQ(4321.1234, arg->value.doublevalue);
}

TEST_F(TEST_FIXTURE, ShouldParseOptionShortString)
{
    int argc = 0;
    const char* defvalue = "1234.4321";
    const char* expvalue = "4321.1234";
    sprintf(gBuffer, "program --s=""4321.1234");
    tokenise_to_argc_argv(gBuffer, &argc, gArgv, ARGV_SIZE, print_arguments);

    gHandle = argsparse_create(NULL);
    argsparse_add_cstr(gHandle, "string", "This is a string", defvalue);
    ARG_ARGUMENT_HANDLE arg = argsparse_argument_by_name(gHandle, "string");
    ASSERT_THAT(arg, NotNull());
    ASSERT_EQ(0, strncmp(defvalue, arg->value.stringvalue, strlen(defvalue)));
    ASSERT_EQ(1, argsparse_parse_args(gHandle, gArgv, argc));
    ASSERT_EQ(1, arg->parsed);
    ASSERT_EQ(0, strncmp(expvalue, arg->value.stringvalue, strlen(expvalue)));
}

TEST_F(TEST_FIXTURE, ShouldParseOptionLongString1)
{
    int argc = 0;
    const char* defvalue = "1234.4321";
    const char* expvalue = "4321.1234";
    sprintf(gBuffer, "program --string=""4321.1234");
    tokenise_to_argc_argv(gBuffer, &argc, gArgv, ARGV_SIZE, print_arguments);

    gHandle = argsparse_create(NULL);
    argsparse_add_cstr(gHandle, "string", "This is a string", defvalue);
    ARG_ARGUMENT_HANDLE arg = argsparse_argument_by_name(gHandle, "string");
    ASSERT_THAT(arg, NotNull());
    ASSERT_EQ(0, strncmp(defvalue, arg->value.stringvalue, strlen(defvalue)));
    ASSERT_EQ(1, argsparse_parse_args(gHandle, gArgv, argc));
    ASSERT_EQ(1, arg->parsed);
    ASSERT_EQ(0, strncmp(expvalue, arg->value.stringvalue, strlen(expvalue)));
}

TEST_F(TEST_FIXTURE, ShouldParseOptionLongString2)
{
    int argc = 0;
    const char* defvalue = "1234.4321";
    const char* expvalue = "4321.1234";
    sprintf(gBuffer, "program --string ""4321.1234");
    tokenise_to_argc_argv(gBuffer, &argc, gArgv, ARGV_SIZE, print_arguments);

    gHandle = argsparse_create(NULL);
    argsparse_add_cstr(gHandle, "string", "This is a string", defvalue);
    ARG_ARGUMENT_HANDLE arg = argsparse_argument_by_name(gHandle, "string");
    ASSERT_THAT(arg, NotNull());
    ASSERT_EQ(0, strncmp(defvalue, arg->value.stringvalue, strlen(defvalue)));
    ASSERT_EQ(1, argsparse_parse_args(gHandle, gArgv, argc));
    ASSERT_EQ(1, arg->parsed);
    ASSERT_EQ(0, strncmp(expvalue, arg->value.stringvalue, strlen(expvalue)));
}

TEST_F(TEST_FIXTURE, ShouldSplitParseWhitespaceString)
{
    int argc = 0;
    const char* defvalue = "1234.4321";
    const char* expvalue = "4321 1234";
    sprintf(gBuffer, "%s%s", "program --string ", expvalue);
    tokenise_to_argc_argv(gBuffer, &argc, gArgv, ARGV_SIZE, print_arguments);

    gHandle = argsparse_create(NULL);
    argsparse_add_cstr(gHandle, "string", "This is a string", defvalue);
    ARG_ARGUMENT_HANDLE arg = argsparse_argument_by_name(gHandle, "string");
    ASSERT_THAT(arg, NotNull());
    ASSERT_EQ(0, strncmp(defvalue, arg->value.stringvalue, strlen(defvalue)));
    ASSERT_EQ(1, argsparse_parse_args(gHandle, gArgv, argc));
    ASSERT_EQ(1, arg->parsed);
    ASSERT_EQ(1, strncmp(expvalue, arg->value.stringvalue, strlen(expvalue)));
    ASSERT_EQ(0, strncmp(expvalue, arg->value.stringvalue, 4));
}

TEST_F(TEST_FIXTURE, ShouldParseDoubleQuotedWhitespaceString)
{
    int argc = 0;
    const char* defvalue = "1234.4321";
    const char* expvalue = "4321 1234";
    char buffer[50] = "program\000--string\0004321 1234";
    gArgv[0] = buffer;
    gArgv[1] = buffer + 8;
    gArgv[2] = buffer + 17;
    argc = 3;
    print_arguments(gArgv, argc);
    gHandle = argsparse_create(NULL);
    argsparse_add_cstr(gHandle, "string", "This is a string", defvalue);
    ARG_ARGUMENT_HANDLE arg = argsparse_argument_by_name(gHandle, "string");
    ASSERT_THAT(arg, NotNull());
    ASSERT_EQ(0, strncmp(defvalue, arg->value.stringvalue, strlen(defvalue)));
    ASSERT_EQ(1, argsparse_parse_args(gHandle, gArgv, argc));
    ASSERT_EQ(1, arg->parsed);
    ASSERT_EQ(0, strncmp(expvalue, arg->value.stringvalue, strlen(expvalue)));
}

// Parametrised test for all types {0,1,2,3}

TEST_P(TEST_FIXTURE, ShouldAddArgument)
{
    ARG_TYPE type = GetParam();
    gHandle = argsparse_create(NULL);
    ASSERT_EQ(0, argsparse_argument_count(gHandle));
    ARG_ARGUMENT_HANDLE h = argsparse_create_argument_with_value(type, "argument", "This is an argument", NULL);
    ASSERT_THAT(h, NotNull());
    argsparse_put_argument(gHandle, &h);
    ASSERT_THAT(h, IsNull());
    ASSERT_EQ(1, argsparse_argument_count(gHandle));
}

TEST_P(TEST_FIXTURE, ShouldInitializeValue)
{
    ARG_TYPE type = GetParam();
    gHandle = argsparse_create(NULL);
    ASSERT_EQ(0, argsparse_argument_count(gHandle));
    ARG_ARGUMENT_HANDLE h = argsparse_create_argument_with_value(type, "flag", "This is an argument", "1234");
    argsparse_put_argument(gHandle, &h);
    ARG_ARGUMENT_HANDLE arg = argsparse_argument_by_name(gHandle, "flag");
    ARG_VALUE value = arg->value;
    switch (type)
    {
        case ARGSPARSE_TYPE_FLAG:
            ASSERT_EQ(value.flagvalue, 1);
        break;
        case ARGSPARSE_TYPE_STRING:
            ASSERT_STREQ(value.stringvalue, "1234");
        break;
        case ARGSPARSE_TYPE_INT:
            ASSERT_EQ(value.intvalue, 1234);
        break;
        case ARGSPARSE_TYPE_DOUBLE:
            ASSERT_DOUBLE_EQ(value.doublevalue, 1234.0);
        break;
        default:
            GTEST_FAIL();
        break;
    }
}

INSTANTIATE_TEST_SUITE_P(
        ArgmentTypesTests,
        TEST_FIXTURE,
        ::testing::Values(
            ARGSPARSE_TYPE_FLAG,
            ARGSPARSE_TYPE_DOUBLE, 
            ARGSPARSE_TYPE_INT,
            ARGSPARSE_TYPE_STRING
        ));
}
