/**
 * @file argsparseTests.cpp
 * @author Tuomas Lahtinen (tuomas123lahtinen@gmail.com)
 * @brief 
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "argsparse.h"

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

ARG_DATA_HANDLE gHandle = nullptr;
class TEST_FIXTURE:public ::testing::TestWithParam<ARG_TYPE>
{
  protected:
    void SetUp() override
    {
        
    }

    void TearDown() override
    {
       argsparse_free(gHandle);
    }
};

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
    for (int i = 0; i <= ARGSPARSE_MAX_ARGUMENTS; i++)
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
    ASSERT_EQ(ARG_ERROR::ERROR_MAX_ARGUMENTS, err);
    ASSERT_EQ(ARGSPARSE_MAX_ARGUMENTS, argsparse_argument_count(gHandle));
}

TEST_F(TEST_FIXTURE, ShouldParseFlag)
{
    gHandle = argsparse_create(NULL);
    argsparse_add_flag(gHandle, "flag", "This is a flag", 0);
    
    const char* argv[2] = {"program", "--flag"};
    int argc = 2;     
    ASSERT_EQ(1, argsparse_parse_args(gHandle, (char* const *)argv, argc));
    ARG_ARGUMENT_HANDLE arg = argsparse_argument_by_name(gHandle, "flag");
    // Flag will not get parsed it was set by the getopts_long
    ASSERT_NE(0, arg->parsed);
}

// Parametrised test for all types {0,1,2,3}

TEST_P(TEST_FIXTURE, ShouldAddArgument) {
    ARG_TYPE type = GetParam();
    gHandle = argsparse_create(NULL);
    ASSERT_EQ(0, argsparse_argument_count(gHandle));
    ARG_ARGUMENT_HANDLE h = argsparse_create_argument_with_value(type, "flag", "This is an argument", NULL);
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
