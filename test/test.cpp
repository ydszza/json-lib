#include "../src/ydsjson.h"
#include <iostream>

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equal, expect, actual) \
    do { \
        test_count++; \
        if (equal) \
            test_pass++; \
        else {\
            std::cout << __FILE__ << ':' << __LINE__ \
                      << ": expect: " << expect \
                      << " actual: " << actual << std::endl; \
            main_ret = 1; \
        } \
    } while (0)

#define EXPECT_EQ(expect, actual) \
    EXPECT_EQ_BASE((expect)==(actual), expect, actual)
#define EXPECT_EQ_STRING(expect, actual, aclen) \
    EXPECT_EQ_BASE(sizeof(expect)-1 == (aclen) && memcmp(expect, actual, aclen) == 0, expect, actual)
#define EXPECT_EQ_TRUE(actual) \
    EXPECT_EQ_BASE((actual) != 0, "true", "false")
#define EXPECT_EQ_FALSE(actual) \
    EXPECT_EQ_BASE((actual) == 0, "false", "true")

#define EXPECT_EQ_SIZE(expect, actual) \
    EXPECT_EQ_BASE((expect)==(actual), expect, actual)

static void test_parse_null() {
    YdsJson json_parse;
    YdsValue value;
    value.set_boolean(false);
    EXPECT_EQ(YDS_PARSE_OK, json_parse.parse(&value, "null"));
    EXPECT_EQ(YDS_NULL, value.get_type());
}

static void test_parse_true() {
    YdsValue value;
    YdsJson json_parse;
    value.set_boolean(false);
    EXPECT_EQ(YDS_PARSE_OK, json_parse.parse(&value, "true"));
    EXPECT_EQ(YDS_TRUE, value.get_type());
}

static void test_parse_false() {
    YdsValue value;
    YdsJson json_parse;
    value.set_boolean(true);
    EXPECT_EQ(YDS_PARSE_OK, json_parse.parse(&value, "false"));
    EXPECT_EQ(YDS_FALSE, value.get_type());
}

#define TEST_NUMBER(EXPECT, json) \
    do { \
        YdsJson json_parse; \
        YdsValue value; \
        EXPECT_EQ(YDS_PARSE_OK, json_parse.parse(&value, json)); \
        EXPECT_EQ(YDS_NUMBER, value.get_type()); \
        EXPECT_EQ(EXPECT, value.get_number()); \
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

#define TEST_STRING(EXPECT, json) \
    do { \
        YdsValue value; \
        YdsJson json_parse; \
        EXPECT_EQ(YDS_PARSE_OK, json_parse.parse(&value, json)); \
        EXPECT_EQ(YDS_STRING, value.get_type()); \
        EXPECT_EQ_STRING(EXPECT, value.get_string(), value.get_string_len()); \
    } while (0)

static void test_parse_string() {
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
    TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
    TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}

static void test_parse_array() {
    YdsValue value;
    YdsJson json_parse;
    EXPECT_EQ(YDS_PARSE_OK, json_parse.parse(&value, "[ ]"));
    EXPECT_EQ(YDS_ARRAY, value.get_type());
    EXPECT_EQ_SIZE(0, value.get_array_size());
    //std::cout << 4 << std::endl;

    value.set_type(YDS_NULL);
    EXPECT_EQ(YDS_PARSE_OK, json_parse.parse(&value, "[ null , false , true , 123 , \"abc\" ]"));
    EXPECT_EQ(YDS_ARRAY, value.get_type());
    EXPECT_EQ_SIZE(5, value.get_array_size());
    EXPECT_EQ(YDS_NULL,   value.get_array_element(0)->get_type());
    EXPECT_EQ(YDS_FALSE,  value.get_array_element(1)->get_type());
    EXPECT_EQ(YDS_TRUE,   value.get_array_element(2)->get_type());
    EXPECT_EQ(YDS_NUMBER, value.get_array_element(3)->get_type());
    EXPECT_EQ(YDS_STRING, value.get_array_element(4)->get_type());
    EXPECT_EQ(123.0, value.get_array_element(3)->get_number());
    EXPECT_EQ_STRING("abc", value.get_array_element(4)->get_string(), 
                     value.get_array_element(4)->get_string_len());

    value.set_type(YDS_NULL);
    EXPECT_EQ(YDS_PARSE_OK, json_parse.parse(&value, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
    EXPECT_EQ(YDS_ARRAY, value.get_type());
    EXPECT_EQ_SIZE(4, value.get_array_size());
    for (int i = 0; i < 4; i++) {
        //std::cout << "i = " << i << " ";
        YdsValue* a = value.get_array_element(i);
        EXPECT_EQ(YDS_ARRAY, a->get_type());
        EXPECT_EQ_SIZE(i, a->get_array_size());
        for (int j = 0; j < i; j++) {
            //std::cout << "j = " << j << " ";
            YdsValue* e = a->get_array_element(j);
            EXPECT_EQ(YDS_NUMBER, e->get_type());
            EXPECT_EQ((double)j,e->get_number());
        }
        //std::cout << std::endl;
    }
}

static void test_parse_object() {
    YdsJson json_parse;
    YdsValue value;
    EXPECT_EQ(YDS_PARSE_OK, json_parse.parse(&value, " { } "));
    EXPECT_EQ(YDS_OBJECT, value.get_type());
    EXPECT_EQ_SIZE(0, value.get_object_size());

    //value.set_type((YDS_NULL));

}

#define TEST_ERROR(error, json) \
    do { \
        YdsValue value; \
        YdsJson json_parse; \
        value.set_boolean(false); \
        EXPECT_EQ(error, json_parse.parse(&value, json)); \
        EXPECT_EQ(YDS_NULL, value.get_type()); \
    } while (0)

static void test_parse_EXPECT_value() {
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

        /* invalid value in array */
    TEST_ERROR(YDS_PARSE_INVALID_VALUE, "[1,]");
    TEST_ERROR(YDS_PARSE_INVALID_VALUE, "[\"a\", nul]");
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

static void test_parse_missing_quotation_mark() {
    TEST_ERROR(YDS_PARSE_MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(YDS_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
    TEST_ERROR(YDS_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(YDS_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(YDS_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(YDS_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
    TEST_ERROR(YDS_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(YDS_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}


static void test_parse_invalid_unicode_hex() {
    TEST_ERROR(YDS_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    TEST_ERROR(YDS_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_ERROR(YDS_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    TEST_ERROR(YDS_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    TEST_ERROR(YDS_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
    TEST_ERROR(YDS_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
    TEST_ERROR(YDS_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(YDS_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
    TEST_ERROR(YDS_PARSE_INVALID_UNICODE_HEX, "\"\\u00/0\"");
    TEST_ERROR(YDS_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
    TEST_ERROR(YDS_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
    TEST_ERROR(YDS_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
    TEST_ERROR(YDS_PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}

static void test_parse_invalid_unicode_surrogate() {
    TEST_ERROR(YDS_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_ERROR(YDS_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TEST_ERROR(YDS_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_ERROR(YDS_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_ERROR(YDS_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

static void test_parse_miss_comma_or_square_bracket() {
    TEST_ERROR(YDS_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
    TEST_ERROR(YDS_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
    TEST_ERROR(YDS_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
    TEST_ERROR(YDS_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_string();
    test_parse_array();

    test_parse_EXPECT_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_number_too_big();
    test_parse_missing_quotation_mark();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();
    test_parse_invalid_unicode_hex();
    test_parse_invalid_unicode_surrogate();
    test_parse_miss_comma_or_square_bracket();
}

static void test_access_null() {
    YdsValue value;
    value.set_string("a", 1);
    value.set_type(YDS_NULL);
    EXPECT_EQ(YDS_NULL, value.get_type());
}

static void test_access_boolean() {
    YdsValue value;
    value.set_string("a", 1);
    value.set_boolean(true);
    EXPECT_EQ_TRUE(value.get_boolean());
    value.set_boolean(false);
    EXPECT_EQ_FALSE(value.get_boolean());
}

static void test_access_number() {
    YdsValue value;
    value.set_string("a", 1);
    value.set_number(1234.5);
    EXPECT_EQ(1234.5, value.get_number());
}

static void test_access_string() {
    YdsValue value;
    value.set_string("", 0);
    EXPECT_EQ_STRING("", value.get_string(), value.get_string_len());
    value.set_string("Hello", 5);
    EXPECT_EQ_STRING("Hello", value.get_string(), value.get_string_len());
}

static void test_access() {
    test_access_null();
    test_access_boolean();
    test_access_number();
    test_access_string();
}

int main() {
    test_parse();
    test_access();
    std::cout << test_pass << "/" << test_count << " "
              << "(" << test_pass * 100.0 / test_count << "%) passed" 
              << std::endl;
    return main_ret;
}