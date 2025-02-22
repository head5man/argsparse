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

class TestParams
{
    int _flagValue = 0;
public:
    static const TestParams StringType;
    static const TestParams IntType;
    static const TestParams DoubleType;
    static const TestParams FlagType;

    TestParams(ARG_TYPE type, const char* string)
    {
        Type = type;
        memcpy(&Value.stringvalue, string, strlen(string));
    }

    TestParams(ARG_TYPE type, double value)
    {
        Type = type;
        Value.doublevalue = value;
    }

    TestParams(ARG_TYPE type, int value)
    {
        Type = type;
        Value.intvalue = value;
    }

    TestParams(ARG_TYPE type, int* value)
    {
        Type = type;
        Value.flagptr = value != nullptr ? value : &_flagValue;
    }

    ARG_TYPE Type;
    ARG_VALUE Value;
};

const TestParams TestParams::StringType(ARGSPARSE_TYPE_STRING, "1234");
const TestParams TestParams::DoubleType(ARGSPARSE_TYPE_DOUBLE, 1234.1);
const TestParams TestParams::IntType(ARGSPARSE_TYPE_INT, 1234);
const TestParams TestParams::FlagType(ARGSPARSE_TYPE_FLAG, (int*)nullptr);

class TEST_FIXTURE:public ::testing::TestWithParam<TestParams>
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
    char* shortopts = argsparse_get_shortopts(gHandle);
    ASSERT_EQ(0, *shortopts);

    ARG_ARGUMENT_HANDLE harg = argsparse_create_argument_with_value(ARG_TYPE::ARGSPARSE_TYPE_INT, "integer", "description", NULL);
    argsparse_put_argument(gHandle, &harg);
    ASSERT_STREQ("i:", shortopts);
}

TEST_F(TEST_FIXTURE, ShouldNotAppendSameOption)
{
    gHandle = argsparse_create(NULL);
    char* shortopts = argsparse_get_shortopts(gHandle);
    ASSERT_EQ(0, *shortopts);

    ARG_ARGUMENT_HANDLE harg = argsparse_create_argument_with_value(ARG_TYPE::ARGSPARSE_TYPE_INT, "integer", "description", NULL);
    argsparse_put_argument(gHandle, &harg);
    ASSERT_THAT(harg, IsNull());
    harg = argsparse_create_argument_with_value(ARG_TYPE::ARGSPARSE_TYPE_INT, "integer", "description", NULL);
    ARG_ERROR err = argsparse_put_argument(gHandle, &harg);
    ASSERT_EQ(ERROR_EXISTS, err);
    ASSERT_STREQ("i:", shortopts);
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

TEST_F(TEST_FIXTURE, ParsesAllOptionTypes)
{
    int flgValue = 0;
    const char* strExpected = "new_value";
    double dblExpected = 4321.4321;
    int flgExpected = 1234;
    int intExpected = 4321;

    gHandle = argsparse_create(NULL);
    argsparse_add_cstr(gHandle, "string", "", "This is the initial value");
    argsparse_add_double(gHandle, "double", "", 1234.1234);
    argsparse_add_flag(gHandle, "flag", "", flgExpected, &flgValue);
    argsparse_add_int(gHandle, "integer", "", 1234);
    const char* fmt = "prg --string %s --double %f --flag --integer %d";
    sprintf(gBuffer, fmt, strExpected, dblExpected, intExpected);
    tokenise_to_argc_argv(gBuffer, &gArgc, gArgv, ARGV_SIZE, print_arguments);
    ASSERT_EQ(4, argsparse_parse_args(gHandle, gArgv, gArgc));

    ARG_ARGUMENT_HANDLE arg;
    arg = argsparse_argument_by_name(gHandle, "string");
    ASSERT_STREQ("new_value", arg->value.stringvalue);
    arg = argsparse_argument_by_name(gHandle, "double");
    ASSERT_DOUBLE_EQ(dblExpected, arg->value.doublevalue);
    ASSERT_EQ(flgExpected, flgValue);
    arg = argsparse_argument_by_name(gHandle, "integer");
    ASSERT_EQ(intExpected, arg->value.intvalue);
}

TEST_F(TEST_FIXTURE, ShouldParseOptionLongFlag)
{
    int argc = 0;
    int value = 0;
    int expected = 1234;
    sprintf(gBuffer, "program --flag");
    tokenise_to_argc_argv(gBuffer, &argc, gArgv, ARGV_SIZE, print_arguments);

    gHandle = argsparse_create(NULL);
    argsparse_add_flag(gHandle, "flag", "This is a flag", expected, &value);
    
    ASSERT_EQ(1, argsparse_parse_args(gHandle, gArgv, argc));

    ARG_ARGUMENT_HANDLE arg = argsparse_argument_by_name(gHandle, "flag");
    ASSERT_EQ(1, arg->parsed);
    ASSERT_EQ(expected, *arg->value.flagptr);
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
    sprintf(gBuffer, "%s%s", "program -s ", expvalue);
    tokenise_to_argc_argv(gBuffer, &argc, gArgv, ARGV_SIZE, print_arguments);

    gHandle = argsparse_create(NULL);
    argsparse_add_cstr(gHandle, "string", "This is a string", defvalue);
    ARG_ARGUMENT_HANDLE arg = argsparse_argument_by_name(gHandle, "string");
    ASSERT_THAT(arg, NotNull());
    ASSERT_EQ(0, strncmp(defvalue, arg->value.stringvalue, strlen(defvalue)));
    ASSERT_EQ(1, argsparse_parse_args(gHandle, gArgv, argc));
    ASSERT_EQ(1, arg->parsed);
    ASSERT_STREQ(expvalue, arg->value.stringvalue);
}

TEST_F(TEST_FIXTURE, ShouldParseOptionLongString1)
{
    int argc = 0;
    const char* defvalue = "1234.4321";
    const char* expvalue = "4321.1234";
    sprintf(gBuffer, "%s%s", "program --string=", expvalue);
    tokenise_to_argc_argv(gBuffer, &argc, gArgv, ARGV_SIZE, print_arguments);

    gHandle = argsparse_create(NULL);
    argsparse_add_cstr(gHandle, "string", "This is a string", defvalue);
    ARG_ARGUMENT_HANDLE arg = argsparse_argument_by_name(gHandle, "string");
    ASSERT_THAT(arg, NotNull());
    ASSERT_EQ(0, strncmp(defvalue, arg->value.stringvalue, strlen(defvalue)));
    ASSERT_EQ(1, argsparse_parse_args(gHandle, gArgv, argc));
    ASSERT_EQ(1, arg->parsed);
    ASSERT_STREQ(expvalue, arg->value.stringvalue);
}

TEST_F(TEST_FIXTURE, ShouldParseOptionLongString2)
{
    int argc = 0;
    const char* defvalue = "1234.4321";
    const char* expvalue = "4321.1234";
    sprintf(gBuffer, "%s%s", "program --string ", "4321.1234");
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

TEST_F(TEST_FIXTURE, UsageOutput)
{
    const char* const executable = "\\some\\path\\test.exe";
    const char* expected =
    "usage: test.exe [-s]\n"
    "title: Title\n"
    "optional arguments:\n"
    "-s, --string\n"
    "    desc: This is a string\n"
    "    args: [str:defvalue]\n"
    "\n";

    gHandle = argsparse_create("Title");

    argsparse_add_cstr(gHandle, "string", "This is a string", "defvalue");
    ::testing::internal::CaptureStdout();
    argsparse_show_usage(gHandle, executable);
    std::string output = ::testing::internal::GetCapturedStdout();
    ASSERT_STREQ(expected, output.c_str());
}

// Parametrised test for all types {0,1,2,3}

TEST_P(TEST_FIXTURE, ShouldAddArgument)
{
    TestParams params = GetParam();
    gHandle = argsparse_create(NULL);
    ASSERT_EQ(0, argsparse_argument_count(gHandle));
    ARG_ARGUMENT_HANDLE h = argsparse_create_argument_with_value(params.Type, "argument", "This is an argument", NULL);
    ASSERT_THAT(h, NotNull());
    argsparse_put_argument(gHandle, &h);
    ASSERT_THAT(h, IsNull());
    ASSERT_EQ(1, argsparse_argument_count(gHandle));
}

TEST_P(TEST_FIXTURE, ShouldInitializeValue)
{
    TestParams param = GetParam();
    gHandle = argsparse_create(NULL);
    ASSERT_EQ(0, argsparse_argument_count(gHandle));

    ARG_ARGUMENT_HANDLE h = argsparse_create_argument_with_value(param.Type, "flag", "This is an argument", &(param.Value));

    argsparse_put_argument(gHandle, &h);
    ARG_ARGUMENT_HANDLE arg = argsparse_argument_by_name(gHandle, "flag");
    ARG_VALUE value = arg->value;
    switch (param.Type)
    {
        case ARGSPARSE_TYPE_FLAG:
            ASSERT_EQ(value.flagptr, param.Value.flagptr);
        break;
        case ARGSPARSE_TYPE_STRING:
            ASSERT_STREQ(value.stringvalue, param.Value.stringvalue);
        break;
        case ARGSPARSE_TYPE_INT:
            ASSERT_EQ(value.intvalue, param.Value.intvalue);
        break;
        case ARGSPARSE_TYPE_DOUBLE:
            ASSERT_DOUBLE_EQ(value.doublevalue, param.Value.doublevalue);
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
            TestParams::FlagType,
            TestParams::DoubleType, 
            TestParams::IntType,
            TestParams::FlagType
        ));
}
