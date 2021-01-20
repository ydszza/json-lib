#include "ydsjson.h"

#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     (((ch) >= '1' && (ch) <= '9'))
#define PUTC(ch)            do { *static_cast<char *>(context_.buff_push(sizeof(char))) = (ch); } while (0)

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
    context_.set_context(p);
    return YDS_PARSE_OK;
}

/**
 * 解析字符串
*/

const char* YdsJson::parse_hex4(const char* p, unsigned* u) {
    int i;
    *u = 0;
    for (i = 0; i < 4; ++i) {
        char ch = *p++;
        *u <<= 4;
        if      (ch >= '0' && ch <= '9') *u |= ch - '0';
        else if (ch >= 'A' && ch <= 'F') *u |= ch - ('A' - 10);
        else if (ch >= 'a' && ch <= 'f') *u |= ch - ('a' - 10);
        else return nullptr;
    }
    return p;
}

void YdsJson::encode_utf8(unsigned u) {
    if (u <= 0x7F) 
        PUTC(u & 0xFF);
    else if (u <= 0x7FF) {
        PUTC(0xC0 | ((u >> 6) & 0xFF));
        PUTC(0x80 | ( u       & 0x3F));
    }
    else if (u <= 0xFFFF) {
        PUTC(0xE0 | ((u >> 12) & 0xFF));
        PUTC(0x80 | ((u >>  6) & 0x3F));
        PUTC(0x80 | ( u        & 0x3F));
    }
    else {
        assert(u <= 0x10FFFF);
        PUTC(0xF0 | ((u >> 18) & 0xFF));
        PUTC(0x80 | ((u >> 12) & 0x3F));
        PUTC(0x80 | ((u >>  6) & 0x3F));
        PUTC(0x80 | ( u        & 0x3F));
    }
}

#define STRING_ERROR(ret) do { context_.set_top(head); return ret; } while(0)

int YdsJson::parse_string_raw(char** str, size_t* len) {
    size_t head = context_.get_top();
    const char* p = context_.get_context() + 1;
    unsigned u, u2;

    while(true) {
        char ch = *p++;
        switch(ch) {
            case '\"':
                *len = context_.get_top() - head;
                *str = static_cast<char *>(context_.buff_pop(*len));
                //value_->set_string(static_cast<const char*>(context_.buff_pop(len)), len);
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
                    case 'u': 
                        if (!(p = parse_hex4(p, &u)))
                            STRING_ERROR(YDS_PARSE_INVALID_UNICODE_HEX);
                        if (u >= 0xD800 && u <= 0xDBFF) {
                            if (*p++ != '\\')
                                STRING_ERROR(YDS_PARSE_INVALID_UNICODE_SURROGATE);
                            if (*p++ != 'u')
                                STRING_ERROR(YDS_PARSE_INVALID_UNICODE_SURROGATE);
                            if (!(p = parse_hex4(p, &u2)))
                                STRING_ERROR(YDS_PARSE_INVALID_UNICODE_HEX);
                            if (u2 < 0xDC00 || u2 > 0xDFFF)
                                STRING_ERROR(YDS_PARSE_INVALID_UNICODE_SURROGATE);
                            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                        }
                        encode_utf8(u);
                        break;
                    default:    
                        STRING_ERROR(YDS_PARSE_INVALID_STRING_ESCAPE);
                }
                break;

            case '\0':
                STRING_ERROR(YDS_PARSE_MISS_QUOTATION_MARK);
            
            default:
                if (static_cast<unsigned char>(ch) < 0x20) {
                    STRING_ERROR(YDS_PARSE_INVALID_STRING_CHAR);
                }
                PUTC(ch);
        }
    }
}

int YdsJson::parse_string() {
    int ret;
    char* s;
    size_t len;
    if ((ret = parse_string_raw(&s, &len)) == YDS_PARSE_OK)
        value_->set_string(s, len);
    return ret;
}

int YdsJson::parse_array() {
    size_t size = 0;
    int ret;
    context_.read_byte();
    parse_whitespace();
    if (*context_.get_context() == ']') {
        context_.read_byte();
        //value_->set_type(YDS_ARRAY);
        value_->set_array(nullptr, 0);
        return YDS_PARSE_OK;
    }

    YdsValue* tmp = value_;
    value_ = static_cast<YdsValue *>(malloc(sizeof(YdsValue)));
    while (true) {
        value_->init();
        if ((ret = parse_value()) != YDS_PARSE_OK) 
            break;
        memcpy(context_.buff_push(sizeof(YdsValue)), value_, sizeof(YdsValue));
        size++;

        parse_whitespace();

        if (*context_.get_context() == ',') {
            context_.read_byte();
            parse_whitespace();
        }
        else if (*context_.get_context() == ']') {
            context_.read_byte();
            //tmp->set_type(YDS_ARRAY);
            //tmp->set_array_size(size);
            //size *= sizeof(YdsValue);
            tmp->set_array(static_cast<char *>(context_.buff_pop(size*sizeof(YdsValue))), size);
            free(value_);
            value_ = tmp;
            return YDS_PARSE_OK;
        }
        else {
            ret = YDS_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
            break;
        }
    }
    free(value_);
    value_ = tmp;
    for (int i = 0; i < size; i++) {
       static_cast<YdsValue *>(context_.buff_pop(sizeof(YdsValue)))->destroy();
    }
    return ret;
}

int YdsJson::parse_object() {
    context_.read_byte();

    //空对象
    parse_whitespace();
    if (*context_.get_context() == '}') {
        context_.read_byte();
        value_->set_object(nullptr, 0, 0);
        return YDS_PARSE_OK;
    }

    size_t size = 0;
    int ret;
    YdsMember m;
    m.key = nullptr;
    //m.v = static_cast<YdsValue *>(malloc(sizeof(YdsValue)));
    while (true) {
        char* str;
        m.v.init();

        /*解析key, 先判断在解析*/
        if (*context_.get_context() != '"') {
            ret = YDS_PARSE_MISS_KEY;
            break;
        }
        if ((ret = parse_string_raw(&str, &m.key_len)) != YDS_PARSE_OK)
            break;
        memcpy(m.key = static_cast<char *>(malloc(m.key_len+1)), str, m.key_len);
        m.key[m.key_len] = '\0';

        parse_whitespace();
        if (*context_.get_context() != ':') {
            ret = YDS_PARSE_MISS_COLON;
            break;
        }
        context_.read_byte();

        /*解析键值*/
        parse_whitespace();
        if ((ret = parse_value()) != YDS_PARSE_OK)
            break;
        memcpy(context_.buff_push(sizeof(YdsMember)), &m, sizeof(YdsMember));
        size++;
        m.key = nullptr;

        parse_whitespace();
        if (*context_.get_context() == ',') {
            context_.read_byte();
            parse_whitespace();
        }
        else if (*context_.get_context() == '}') {
            context_.read_byte();
            value_->set_object(static_cast<char *>(context_.buff_pop(size*sizeof(YdsMember))), sizeof(YdsMember)*size, size);
            return YDS_PARSE_OK;
        }
        else {
            ret = YDS_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
            break;
        }
    }

    free(m.key);
    for (int i = 0; i < size; ++i) {
        YdsMember* m = static_cast<YdsMember *>(context_.buff_pop(sizeof(YdsMember)));
        free(m->key);
        m->v.destroy();//?
    }
    value_->set_type(YDS_NULL);
    return ret;
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

        case '[':
            return parse_array();
        
        case '{':
            return parse_object();

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
    assert(context_.get_top() == 0);
    return ret; //解析失败或解析到末尾
}