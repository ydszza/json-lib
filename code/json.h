#ifndef __JSON_H__
#define __JSON_H__

#include "value.h"
#include <errno.h>
#include <math.h>


/**
 * 定义解析结果返回值
*/
enum {
    PARSE_OK = 0,                       /*解析成功*/
    PARSE_EXPECT_VALUE,                 /*全是空白*/
    PARSE_INVALID_VALUE,                /*无效的值*/
    PARSE_ROOT_NOT_SINGULAR,            /*解析之后还有未解析完的数据*/

    PARSE_NUMBER_TOO_BIG,               /*解析的数字数值太大*/
    
    PARSE_INVALID_STRING_ESCAPE,        /*字符串解析出错*/
    PARSE_MISS_QUOTATION_MARK,          /*字符串不完整*/
    PARSE_INVALID_STRING_CHAR,          /*包含无效的字符*/
    PARSE_INVALID_UNICODE_HEX,          /*无效的Unicode十六进制码*/
    PARSE_INVALID_UNICODE_SURROGATE,    /*无效的代理码*/

    PARSE_MISS_COMMA_OR_SQUARE_BRACKET, /*数组解析出错(逗号或方括号缺少)*/

    PARSE_MISS_KEY,                     /*缺少键*/
    PARSE_MISS_COLON,                   /*缺少冒号*/
    PARSE_MISS_COMMA_OR_CURLY_BRACKET,  /*缺少圆括号*/
};

class Json {
public:
    Json() : value_(std::make_shared<Value>()) {}
    int parse(const char* json, Value::ValuePtr& value);
    void stringify(Value::ValuePtr& value, std::string& str);

private:
    int parse_value();
    void parse_whitespace();
    int parse_literial(std::string literal, value_type type);
    int parse_number();
    int parse_string();
    int parse_string_raw(std::string& str);
    unsigned parse_hex4();
    std::string encode_utf8(unsigned u);
    int parse_array();
    int parse_object();

    bool is_digit(char ch) { return ch >= '0' && ch <= '9'; }
    bool is_digit_1to9(char ch) { return ch >= '1' && ch <= '9'; }

    void stringify_string(Value::ValuePtr& value, std::string& str);
    void stringify_value(Value::ValuePtr& value, std::string& str, size_t level);

private:
    const char* json_;
    Value::ValuePtr value_;
};

#endif // !__JSON_H__