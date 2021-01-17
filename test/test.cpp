#include "../src/ydsjson.h"
#include <iostream>

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXCEPT_EQ_BASE(equal, except, actual) \
    do { \
        test_count++; \
        if (equal) \
            test_pass++; \
        else {\
            std::cout << __FILE__ << ':' << __LINE__ \
                      << ": except: " << except \
                      << " actual: " << actual << std::endl; \
            main_ret = 1; \
        } \
    } while (0)

#define EXCEPT_EQ(except, actual) \
    EXCEPT_EQ_BASE((except)==(actual), except, actual)

static void test_parse_null() {
    YdsJson json_parse;
    YdsValue value;
    EXCEPT_EQ(YDS_PARSE_OK, json_parse.parse(&value, "null"));
    EXCEPT_EQ(YDS_NULL, value.get_type());
}

static void test_parse_true() {
    YdsValue value;
    YdsJson json_parse;
    EXCEPT_EQ(YDS_PARSE_OK, json_parse.parse(&value, "true"));
    EXCEPT_EQ(YDS_TRUE, value.get_type());
}

static void test_parse_false() {
    YdsValue value;
    YdsJson json_parse;
    EXCEPT_EQ(YDS_PARSE_OK, json_parse.parse(&value, "false"));
    EXCEPT_EQ(YDS_FALSE, value.get_type());
}

static void test_parse_except_value() {
    YdsValue value;
    YdsJson json_parse;
    EXCEPT_EQ(YDS_PARSE_EXPECT_VALUE, json_parse.parse(&value, ""));
    EXCEPT_EQ(YDS_NULL, value.get_type());

    EXCEPT_EQ(YDS_PARSE_EXPECT_VALUE, json_parse.parse(&value, " "));
    EXCEPT_EQ(YDS_NULL, value.get_type());
}

static void test_parse_invalid_value() {
    YdsValue value;
    YdsJson json_parse;
    EXCEPT_EQ(YDS_PARSE_INVALID_VALUE, json_parse.parse(&value, "nul"));
    EXCEPT_EQ(YDS_NULL, value.get_type());
    
    EXCEPT_EQ(YDS_PARSE_INVALID_VALUE, json_parse.parse(&value, "?"));
    EXCEPT_EQ(YDS_NULL, value.get_type());
}

static void test_parse_root_not_singular() {
    YdsValue value;
    YdsJson json_parse;
    EXCEPT_EQ(YDS_PARSE_ROOT_NOT_SINGULAR, json_parse.parse(&value, "null x"));
    EXCEPT_EQ(YDS_NULL, value.get_type());
}

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_except_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
}

int main() {
    test_parse();
    std::cout << test_pass << "/" << test_count << " "
              << "(" << test_pass * 100.0 / test_count << "%) passed" 
              << std::endl;
    return main_ret;
}