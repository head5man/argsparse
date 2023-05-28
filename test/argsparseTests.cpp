/**
 * @file argsparseTests.cpp
 * @author Tuomas Lahtinen (tuomas123lahtinen@gmail.com)
 * @brief 
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "../argsparse.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gtest/gtest-matchers.h"
#include <sstream>
#include <ostream>
#include <memory>

namespace skeleton::testing
{
#define TEST_FIXTURE argsparse
using ::testing::NotNull;
using ::testing::IsNull;

HARGPARSE_HANDLE gHandle = nullptr;
class TEST_FIXTURE:public ::testing::TestWithParam<tArgsparseValuetype>
{
  protected:
    void SetUp() override
    {
        
    }

    void TearDown() override
    {
       argsparse_delete(gHandle);
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
    HARGPARSE_ARG harg = argsparse_create_argument(ARGSPARSE_TYPE_INT, "flag", "description", "1234");
    std::cerr << "4" << std::endl;
    argsparse_put_argument(gHandle, &harg);
    std::cerr << "5" << std::endl;
    ASSERT_STREQ("f:", shortopts);
}

TEST_F(TEST_FIXTURE, ShouldAddManyArguments)
{
    gHandle = argsparse_create(NULL);
    char flag[] = { 'f', 'l', 'a', 'g', '?','?','?', 0 };
    const char* fmt = "flag%d";
    for (int i = 0; i < 100; i++)
    {
        sprintf(flag, fmt, i);
        HARGPARSE_ARG h = argsparse_create_argument(
            (tArgsparseValuetype)(i % ARGSPARSE_TYPE_CNT),
            flag, 
            "This is one of the many flags created", 
            NULL);
        int err = argsparse_put_argument(gHandle, &h);
        if (err)
            break;
    }
    ASSERT_EQ(ARGSPARSE_MAX_ARGUMENTS, argsparse_argument_count(gHandle));
}

TEST_F(TEST_FIXTURE, ShouldParseFlag)
{
    gHandle = argsparse_create(NULL);
    argsparse_add_flag(gHandle, "flag", "This is a flag");
    char commandline[100] = "program --flag";
    
    const char* argv[2];
    int argc = 0;
    char* tok = commandline;
    while (tok)
    {
        tok += (int)((argc == 0) ? 0 : 1);
        argv[argc++] = tok;
        tok = strchr(tok, ' ');
    }
     
    ASSERT_EQ(1, argsparse_parse_args(gHandle, (char* const *)argv, argc));
    HARGPARSE_ARG arg = argsparse_argument_by_name(gHandle, "flag");
    ASSERT_NE(0, arg->parsed);
}

// Parametrised test for all types {0,1,2,3}

TEST_P(TEST_FIXTURE, ShouldAddArgument) {
    tArgsparseValuetype type = GetParam();
    gHandle = argsparse_create(NULL);
    ASSERT_EQ(0, argsparse_argument_count(gHandle));
    HARGPARSE_ARG h = argsparse_create_argument(type, "flag", "This is an argument", NULL);
    ASSERT_THAT(h, NotNull());
    argsparse_put_argument(gHandle, &h);
    ASSERT_THAT(h, IsNull());
    ASSERT_EQ(1, argsparse_argument_count(gHandle));
}

TEST_P(TEST_FIXTURE, ShouldInitializeValue)
{
    tArgsparseValuetype type = GetParam();
    gHandle = argsparse_create(NULL);
    ASSERT_EQ(0, argsparse_argument_count(gHandle));
    HARGPARSE_ARG h = argsparse_create_argument(type, "flag", "This is an argument", "1234");
    argsparse_put_argument(gHandle, &h);
    HARGPARSE_ARG arg = argsparse_argument_by_name(gHandle, "flag");
    HARGPARSE_VALUE value = arg->value;
    switch (type)
    {
        case ARGSPARSE_TYPE_STRING:
            ASSERT_THAT(value, NotNull());
            ASSERT_STREQ(value->stringvalue, "1234");
        break;
        case ARGSPARSE_TYPE_INT:
            ASSERT_THAT(value, NotNull());
            ASSERT_EQ(value->intvalue, 1234);
        break;
        case ARGSPARSE_TYPE_DOUBLE:
            ASSERT_THAT(value, NotNull());
            ASSERT_DOUBLE_EQ(value->doublevalue, 1234.0);
        break;
        default:
            ASSERT_THAT(value, IsNull());
        break;
    }
}

INSTANTIATE_TEST_CASE_P(
        ArgmentTypesTests,
        TEST_FIXTURE,
        ::testing::Values(
            ARGSPARSE_TYPE_DOUBLE, 
            ARGSPARSE_TYPE_INT,
            ARGSPARSE_TYPE_NONE,
            ARGSPARSE_TYPE_STRING
        ));
}
