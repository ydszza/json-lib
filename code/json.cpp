#include "json.h"


void Json::parse_whitespace() {
    while (*json_ == ' ' || *json_ == '\n' || *json_ == '\r' || *json_ == '\t') 
        json_++;
}

int Json::parse_literial(std::string json, value_type type) {
    const char* p = json_;
    for (auto ch : json) {
        if (ch != *p++) return PARSE_INVALID_VALUE; 
    }
    json_ = p;
    value_->set_type(type);
    return PARSE_OK;
}

int Json::parse_number() {
    const char* p = json_;
    if (*p == '-') p++;
    if (*p == '0') p++;
    else {
        if (!is_digit_1to9(*p)) return PARSE_INVALID_VALUE;
        for (p++; is_digit(*p); p++);
    } 
    if (*p == '.') {
        p++;
        if (!is_digit(*p)) return PARSE_INVALID_VALUE;
        for (p++; is_digit(*p); p++);
    }
    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') p++;
        if (!is_digit(*p)) return PARSE_INVALID_VALUE;
        for (p++; is_digit(*p); p++);
    }

    errno = 0;
    double num = strtod(json_, nullptr);
    if (errno == ERANGE && (num == HUGE_VAL || num == -HUGE_VAL)) {
        value_->set_null();
        return PARSE_NUMBER_TOO_BIG;
    }
    value_->set_number(num);
    json_ = p;
    return PARSE_OK;
}

int Json::parse_string() {
    int ret;
    std::string str;
    if ((ret = parse_string_raw(str)) == PARSE_OK)
        value_->set_string(str.data());
    return ret;
}

int Json::parse_string_raw(std::string& str) {
    json_++;
    unsigned u, u2;

    while(true) {
        char ch = *json_++;
        switch(ch) {
            case '\"':
                return  PARSE_OK;
            
            case '\\':
                switch(*json_++) {
                    case '\"':  str += '\"'; break;
                    case '\\':  str += '\\'; break;
                    case '/':   str += '/'; break;
                    case 'b':   str += '\b'; break;
                    case 'f':   str += '\f'; break;
                    case 'n':   str += '\n'; break;
                    case 'r':   str += '\r'; break;
                    case 't':   str += '\t'; break;
                    case 'u': 
                        if ((u = parse_hex4()) == 0x10000)
                            return PARSE_INVALID_UNICODE_HEX;
                        if (u >= 0xD800 && u <= 0xDBFF) {
                            if (*json_++ != '\\')
                                return PARSE_INVALID_UNICODE_SURROGATE;
                            if (*json_++ != 'u')
                                return PARSE_INVALID_UNICODE_SURROGATE;
                            if ((u2 = parse_hex4()) == 0x10000)
                                return PARSE_INVALID_UNICODE_HEX;
                            if (u2 < 0xDC00 || u2 > 0xDFFF)
                                return PARSE_INVALID_UNICODE_SURROGATE;
                            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                        }
                        str += encode_utf8(u);
                        break;
                    default:    
                        return PARSE_INVALID_STRING_ESCAPE;
                }
                break;

            case '\0':
                return PARSE_MISS_QUOTATION_MARK;
            
            default:
                if (static_cast<unsigned char>(ch) < 0x20) {
                    return PARSE_INVALID_STRING_CHAR;
                }
                str += ch;
        }
    }
}

unsigned Json::parse_hex4() {
    int i;
    unsigned u = 0;
    for (i = 0; i < 4; ++i) {
        char ch = *json_++;
        u <<= 4;
        if      (ch >= '0' && ch <= '9') u |= ch - '0';
        else if (ch >= 'A' && ch <= 'F') u |= ch - ('A' - 10);
        else if (ch >= 'a' && ch <= 'f') u |= ch - ('a' - 10);
        else return 0x10000;
    }
    return u;
}

std::string Json::encode_utf8(unsigned u) {
    std::string str;
    if (u <= 0x7F) 
        str += static_cast<char>(u & 0xFF);
    else if (u <= 0x7FF) {
        str += static_cast<char>(0xC0 | ((u >> 6) & 0xFF));
        str += static_cast<char>(0x80 | ( u       & 0x3F));
    }
    else if (u <= 0xFFFF) {
        str += static_cast<char>(0xE0 | ((u >> 12) & 0xFF));
        str += static_cast<char>(0x80 | ((u >>  6) & 0x3F));
        str += static_cast<char>(0x80 | ( u        & 0x3F));
    }
    else {
        assert(u <= 0x10FFFF);
        str += static_cast<char>(0xF0 | ((u >> 18) & 0xFF));
        str += static_cast<char>(0x80 | ((u >> 12) & 0x3F));
        str += static_cast<char>(0x80 | ((u >>  6) & 0x3F));
        str += static_cast<char>(0x80 | ( u        & 0x3F));
    }
    return str;
}

int Json::parse_array() {
    json_++;
    int ret;
    std::vector<Value::ValuePtr> v;
    parse_whitespace();
    if (*json_ == ']') {
        json_++;
        value_->set_array(v);
        return PARSE_OK;
    }
    Value::ValuePtr tmp = value_;
    while (true) {
        value_ = std::make_shared<Value>();
        if ((ret = parse_value()) != PARSE_OK) break;
        v.push_back(value_);

        parse_whitespace();
        if (*json_ == ',') {
            json_++;
            parse_whitespace();
        }
        else if (*json_ == ']') {
            json_++;
            tmp->set_array(v);
            value_ = tmp;
            return PARSE_OK;
        }
        else {
            ret = PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
            value_ = tmp;
            return PARSE_OK;
        }
    }
    value_ = tmp;
    return ret;
}

int Json::parse_object() {
    return PARSE_OK;
}


int Json::parse_value() {
    const char* p = json_;

    switch (*p) {
        case 'n':
            return parse_literial("null", NULL_VALUE);

        case 't':
            return parse_literial("true", TRUE_VALUE);

        case 'f':
            return parse_literial("false", FALSE_VALUE);

        default:
            return parse_number();
        
        case '"':
            return parse_string();

        case '[':
            return parse_array();
        
        case '{':
            return parse_object();

        case '\0': 
            return PARSE_EXPECT_VALUE;
    }
}

int Json::parse(const char* json, Value::ValuePtr& value) {
    json_ = json;
    value_->set_null();

    value = value_;
    int ret;
    parse_whitespace();
    if ((ret = parse_value()) == PARSE_OK) {    //解析成功
        parse_whitespace();
        if (*json_ != '\0') { //解析成功但不是末尾
            value->set_null();
            return PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret; //解析失败或解析到末尾
}