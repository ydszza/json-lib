#include "ydsjson.h"

/**
 * 解析空白部分
 * 直接跳过
*/
void YdsJson::parse_whitespace() {
    const char* p = context_.get_context();
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    context_.set_context(p); 
}

/**
 * 解析字面量，null，bool
*/
int YdsJson::parse_literial(const char* literal, yds_type type) {
    const char* p = context_.get_context();

    size_t i;
    for (i = 1; literal[i]; ++i) {
        if (literal[i] != p[i]) return YDS_PARSE_INVALID_VALUE;
    }
    
    context_.set_context(p+i);
    value_.set_type(type);
    return YDS_PARSE_OK;
}

/**
 * 根据类型解析数据
*/
int YdsJson::parse_value() {
    const char* p = context_.get_context();
    switch (*p) {
        case 'n':
            return parse_literial("null", YDS_NULL);

        case 't':
            return parse_literial("true", YDS_TRUE);

        case 'f':
            return parse_literial("false", YDS_FALSE);

        case '\0': 
            return YDS_PARSE_EXPECT_VALUE;

        default:
            return YDS_PARSE_INVALID_VALUE;
    }
}

/**
 * 解析json数据
*/
int YdsJson::parse(const char* json) {
    assert(json);
    context_.set_context(json);
    
    int ret;
    parse_whitespace();
    if ((ret = parse_value()) == YDS_PARSE_OK) {    //解析成功
        parse_whitespace();
        if (context_.get_context() != '\0') { //解析成功但不是末尾
            return YDS_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret; //解析失败或解析到末尾
}