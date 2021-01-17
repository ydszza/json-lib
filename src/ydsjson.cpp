#include "ydsjson.h"

#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     (((ch) >= '1' && (ch) <= '9'))

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
    value_->set_type(type);
    return YDS_PARSE_OK;
}

/**
 * 解析数字
*/
int YdsJson::parse_number() {
    const char* p = context_.get_context();
    if (*p == '-') p++;
    if (*p == '0') p++;
    else {
        if (!ISDIGIT1TO9(*p)) return YDS_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == '.') {
        p++;
        if (!ISDIGIT(*p)) return YDS_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') p++;
        if (!ISDIGIT(*p)) return YDS_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }

    errno = 0;
    value_->set_number(strtod(context_.get_context(), NULL));
    if (errno == ERANGE && 
        (value_->get_number() == HUGE_VAL || value_->get_number() == -HUGE_VAL)) {
        return YDS_PARSE_NUMBER_TOO_BIG;
    }
    value_->set_type(YDS_NUMBER);
    context_.set_context(p);
    return YDS_PARSE_OK;
}

/**
 * 根据类型解析数据
*/
int YdsJson::parse_value() {
    value_->set_type(YDS_NULL);
    const char* p = context_.get_context();

    switch (*p) {
        case 'n':
            return parse_literial("null", YDS_NULL);

        case 't':
            return parse_literial("true", YDS_TRUE);

        case 'f':
            return parse_literial("false", YDS_FALSE);

        default:
            return parse_number();

        case '\0': 
            return YDS_PARSE_EXPECT_VALUE;
    }
}

/**
 * 解析json数据
*/
int YdsJson::parse(YdsValue* value, const char* json) {
    assert(json && value);
    context_.set_context(json);
    value_ = value;
    
    int ret;
    parse_whitespace();
    if ((ret = parse_value()) == YDS_PARSE_OK) {    //解析成功
        parse_whitespace();
        if (*context_.get_context() != '\0') { //解析成功但不是末尾
            return YDS_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret; //解析失败或解析到末尾
}