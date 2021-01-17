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

#define TEST_NUMBER(except, json) \
    do { \
        YdsJson json_parse; \
        YdsValue value; \
        EXCEPT_EQ(YDS_PARSE_OK, json_parse.parse(&value, json)); \
        EXCEPT_EQ(YDS_NUMBER, value.get_type()); \
        EXCEPT_EQ(except, value.get_number()); \
    } while (0)

static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

#define TEST_ERROR(error, json) \
    do { \
        YdsValue value; \
        YdsJson json_parse; \
        EXCEPT_EQ(error, json_parse.parse(&value, json)); \
        EXCEPT_EQ(YDS_NULL, value.get_type()); \
    } while (0)

static void test_parse_except_value() {
    TEST_ERROR(YDS_PARSE_EXPECT_VALUE, "");
    TEST_ERROR(YDS_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
    TEST_ERROR(YDS_PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(YDS_PARSE_INVALID_VALUE, "?");

    TEST_ERROR(YDS_PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(YDS_PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(YDS_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(YDS_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(YDS_PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(YDS_PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(YDS_PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(YDS_PARSE_INVALID_VALUE, "nan");
}

static void test_parse_root_not_singular() {
    TEST_ERROR(YDS_PARSE_ROOT_NOT_SINGULAR, "null x");

    
    TEST_ERROR(YDS_PARSE_ROOT_NOT_SINGULAR, "0123");
    TEST_ERROR(YDS_PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(YDS_PARSE_ROOT_NOT_SINGULAR, "0x123");
}

static void test_parse_number_too_big() {
    TEST_ERROR(YDS_PARSE_NUMBER_TOO_BIG, "1E309");
    TEST_ERROR(YDS_PARSE_NUMBER_TOO_BIG, "-1E309");
}

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_except_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_number_too_big();
}

int main() {
    test_parse();
    std::cout << test_pass << "/" << test_count << " "
              << "(" << test_pass * 100.0 / test_count << "%) passed" 
              << std::endl;
    return main_ret;
}