#include "ydsjson.h"

#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     (((ch) >= '1' && (ch) <= '9'))
#define PUTC(ch)            do { *static_cast<char *>(context_.buff_push(sizeof(ch))) = (ch); } while (0)

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
        value_->set_type(YDS_NULL);
        return YDS_PARSE_NUMBER_TOO_BIG;
    }
    //value_->set_type(YDS_NUMBER);
    context_.set_context(p);
    return YDS_PARSE_OK;
}

/**
 * 解析字符串
*/
int YdsJson::parse_string() {
    size_t head = context_.get_top(), len;
    const char* p = context_.get_context() + 1;

    while(true) {
        char ch = *p++;
        switch(ch) {
            case '\"':
                len = context_.get_top() - head;
                value_->set_string(static_cast<const char*>(context_.buff_pop(len)), len);
                context_.set_context(p);
                return  YDS_PARSE_OK;
            
            case '\\':
                switch(*p++) {
                    case '\"':  PUTC('\"'); break;
                    case '\\':  PUTC('\\'); break;
                    case '/':   PUTC('/'); break;
                    case 'b':   PUTC('\b'); break;
                    case 'f':   PUTC('\f'); break;
                    case 'n':   PUTC('\n'); break;
                    case 'r':   PUTC('\r'); break;
                    case 't':   PUTC('\t'); break;
                    default:    context_.set_top(head); 
                                return YDS_PARSE_INVALID_STRING_ESCAPE;
                }
                break;

            case '\0':
                context_.set_top(head); 
                return YDS_PARSE_MISS_QUOTATION_MARK;
            
            default:
                if (static_cast<unsigned char>(ch) < 0x20) {
                    context_.set_top(head); 
                    return YDS_PARSE_INVALID_STRING_CHAR;
                }
                PUTC(ch);
        }
    }
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

        default:
            return parse_number();
        
        case '"':
            return parse_string();

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
    value_->set_type(YDS_NULL);
    
    int ret;
    parse_whitespace();
    if ((ret = parse_value()) == YDS_PARSE_OK) {    //解析成功
        parse_whitespace();
        if (*context_.get_context() != '\0') { //解析成功但不是末尾
            value->set_type(YDS_NULL);
            return YDS_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret; //解析失败或解析到末尾
}