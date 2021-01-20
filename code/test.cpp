#include "value.h"
#include "json.h"
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

#define EXPECT_EQ(expect, actual) EXPECT_EQ_BASE((expect)==(actual), expect, actual)
#define EXPECT_EQ_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false")
#define EXPECT_EQ_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true")

/**************************************
 * 合法json解析测试
 **************************************/

static void test_parse_null() {
    Json json;
    Value::ValuePtr value = std::make_shared<Value>();
    //value->set_boolean(false);
    EXPECT_EQ(PARSE_OK, json.parse("null", value));
    EXPECT_EQ(NULL_VALUE, value->get_type());
}

static void test_parse_true() {
    Json json;
    Value::ValuePtr value = std::make_shared<Value>();
    //value->set_boolean(false);
    EXPECT_EQ(PARSE_OK, json.parse("true", value));
    EXPECT_EQ(TRUE_VALUE, value->get_type());
}

static void test_parse_false() {
    Json json;
    Value::ValuePtr value = std::make_shared<Value>();
    //value->set_boolean(true);
    EXPECT_EQ(PARSE_OK, json.parse("false", value));
    EXPECT_EQ(FALSE_VALUE, value->get_type());
}


#define TEST_NUMBER(expect, jso) \
    do { \
        Json json; \
        Value::ValuePtr value = std::make_shared<Value>(); \
        EXPECT_EQ(PARSE_OK, json.parse(jso, value)); \
        EXPECT_EQ(NUMBER_VALUE, value->get_type()); \
        EXPECT_EQ(expect, value->get_number()); \
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


#define TEST_STRING(expect, jso) \
    do { \
        Value::ValuePtr value = std::make_shared<Value>(); \
        Json json; \
        EXPECT_EQ(PARSE_OK, json.parse(jso, value)); \
        EXPECT_EQ(STRING_VALUE, value->get_type()); \
        EXPECT_EQ(expect, value->get_string()); \
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
    Value::ValuePtr value = std::make_shared<Value>();
    Json json;
    EXPECT_EQ(PARSE_OK, json.parse("[ ]", value));
    EXPECT_EQ(ARRAY_VALUE, value->get_type());
    EXPECT_EQ(0, value->get_array().size());

    value->set_null();
    EXPECT_EQ(PARSE_OK, json.parse("[ null , false , true , 123 , \"abc\" ]", value));
    EXPECT_EQ(ARRAY_VALUE, value->get_type());
    EXPECT_EQ(5, value->get_array().size());
    EXPECT_EQ(NULL_VALUE,   value->get_array()[0]->get_type());
    EXPECT_EQ(FALSE_VALUE,  value->get_array()[1]->get_type());
    EXPECT_EQ(TRUE_VALUE,   value->get_array()[2]->get_type());
    EXPECT_EQ(NUMBER_VALUE, value->get_array()[3]->get_type());
    EXPECT_EQ(STRING_VALUE, value->get_array()[4]->get_type());
    EXPECT_EQ(123.0, value->get_array()[3]->get_number());
    EXPECT_EQ("abc", value->get_array()[4]->get_string());

    value->set_null();
    EXPECT_EQ(PARSE_OK, json.parse("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]", value));
    EXPECT_EQ(ARRAY_VALUE, value->get_type());
    EXPECT_EQ(4, value->get_array().size());
    for (int i = 0; i < 4; i++) {
        Value::ValuePtr a = value->get_array()[i];
        EXPECT_EQ(ARRAY_VALUE, a->get_type());
        EXPECT_EQ(i, a->get_array().size());
        for (int j = 0; j < i; j++) {
            Value::ValuePtr e = a->get_array()[j];
            EXPECT_EQ(NUMBER_VALUE, e->get_type());
            EXPECT_EQ((double)j,e->get_number());
        }
    }
}

#if 0
static void test_parse_object() {
    YdsJson json_parse;
    YdsValue::ValuePtr value;
    EXPECT_EQ(PARSE_OK, json_parse.parse(&value, " { } "));
    EXPECT_EQ(OBJECT, value->get_type());
    EXPECT_EQ_SIZE(0, value->get_object_size());

    //value->set_type((NULL));

}
#endif

/*******************************
 * 测试错误数据
 * *****************************/
#define TEST_ERROR(error, jso) \
    do { \
        Value::ValuePtr value = std::make_shared<Value>(); \
        Json json; \
        value->set_boolean(false); \
        EXPECT_EQ(error, json.parse(jso, value)); \
        EXPECT_EQ(NULL_VALUE, value->get_type()); \
    } while (0)

static void test_parse_expect_value() {
    TEST_ERROR(PARSE_EXPECT_VALUE, "");
    TEST_ERROR(PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
    TEST_ERROR(PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(PARSE_INVALID_VALUE, "?");

    TEST_ERROR(PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(PARSE_INVALID_VALUE, "nan");

        /* invalid value in array */
    TEST_ERROR(PARSE_INVALID_VALUE, "[1,]");
    TEST_ERROR(PARSE_INVALID_VALUE, "[\"a\", nul]");
}


static void test_parse_root_not_singular() {
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "null x");

    
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0123");
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0x123");
}

static void test_parse_number_too_big() {
    TEST_ERROR(PARSE_NUMBER_TOO_BIG, "1E309");
    TEST_ERROR(PARSE_NUMBER_TOO_BIG, "-1E309");
}

static void test_parse_missing_quotation_mark() {
    TEST_ERROR(PARSE_MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
    TEST_ERROR(PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}


static void test_parse_invalid_unicode_hex() {
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u00/0\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}

static void test_parse_invalid_unicode_surrogate() {
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

#if 0
static void test_parse_miss_comma_or_square_bracket() {
    TEST_ERROR(PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
    TEST_ERROR(PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
    TEST_ERROR(PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
    TEST_ERROR(PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}
#endif

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_string();
    test_parse_array();

    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_number_too_big();
    test_parse_missing_quotation_mark();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();
    test_parse_invalid_unicode_hex();
    test_parse_invalid_unicode_surrogate();
    //test_parse_miss_comma_or_square_bracket();
}


/**************************************
 * value存取测试
 **************************************/

static void test_access_null() {
    Value::ValuePtr value = std::make_shared<Value>();
    value->set_string("a");
    value->set_null();
    EXPECT_EQ(NULL_VALUE, value->get_type());
}

static void test_access_boolean() {
    Value::ValuePtr value = std::make_shared<Value>();
    value->set_string("a");
    value->set_boolean(true);
    EXPECT_EQ_TRUE(value->get_boolean());
    value->set_boolean(false);
    EXPECT_EQ_FALSE(value->get_boolean());
}

static void test_access_number() {
    Value::ValuePtr value = std::make_shared<Value>();
    value->set_string("a");
    value->set_number(1234.5);
    EXPECT_EQ(1234.5, value->get_number());
}

static void test_access_string() {
    Value::ValuePtr value = std::make_shared<Value>();
    value->set_string("");
    EXPECT_EQ("", value->get_string());
    value->set_string("Hello");
    EXPECT_EQ("Hello", value->get_string());
}

static void test_access() {
    test_access_null();
    test_access_boolean();
    test_access_number();
    test_access_string();
}

int main() { 
    //std::cout << sizeof(Value::ValuePtr) << std::endl;
    test_parse();
    test_access();
    std::cout << test_pass << "/" << test_count << " "
              << "(" << test_pass * 100.0 / test_count << "%) passed" 
              << std::endl;
    return main_ret;
}