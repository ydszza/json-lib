#include "json.h"

/****************************************************************
 * 解析json字符串对象
 * *************************************************************/
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
            break;
        }
    }
    value_ = tmp;
    return ret;
}

int Json::parse_object() {
    json_++;

    //空对象
    std::unordered_map<std::string, Value::ValuePtr> obj;
    parse_whitespace();
    if (*json_ == '}') {
        json_++;
        value_->set_object(obj);
        return PARSE_OK;
    }

    int ret;
    Value::ValuePtr tmp = value_;
    while (true) {
        value_ = std::make_shared<Value>();
        /*解析key, 先判断在解析*/
        if (*json_ != '"') {
            ret = PARSE_MISS_KEY;
            break;
        }
        std::string str;
        if ((ret = parse_string_raw(str)) != PARSE_OK)
            break;
        parse_whitespace();
        if (*json_ != ':') {
            ret = PARSE_MISS_COLON;
            break;
        }
        json_++;
        /*解析键对应的值*/
        parse_whitespace();
        if ((ret = parse_value()) != PARSE_OK)
            break;
        obj[str] = value_;

        parse_whitespace();
        if (*json_ == ',') {
            json_++;
            parse_whitespace();
        }
        else if (*json_ == '}') {
            json_++;
            tmp->set_object(obj);
            value_ = tmp;
            return PARSE_OK;
        }
        else {
            ret = PARSE_MISS_COMMA_OR_CURLY_BRACKET;
            break;
        }
    }
    value_ = tmp;
    return ret;
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


/****************************************************************
 * json对象字符串化
 * *************************************************************/
void Json::stringify_string(Value::ValuePtr& value, std::string& str) {
    static const char hex_digits[] = "0123456789ABCDEF";
    str += '"';
    std::string& s = value->get_string();
    for (auto c : s) {
        switch (c) {
            case '\"': str += "\\\""; break;
            case '\\': str += "\\"; break;
            case '\b': str += "\\b"; break;
            case '\f': str += "\\f"; break;
            case '\n': str += "\\n"; break;
            case '\r': str += "\\r"; break;
            case '\t': str += "\\t"; break;
            default:
                if (c < 0x20) {
                    str += "\\u00";
                    str += hex_digits[c >> 4];//取高四位值
                    str += hex_digits[c & 15];//取低四位
                }
                else str += c;
        }
    }
    str += '"';
}

void Json::stringify_value(Value::ValuePtr& value, std::string& str, size_t level) {
    switch(value->get_type()) {
        case NULL_VALUE:    str += "null"; break;
        case TRUE_VALUE:    str += "true"; break;
        case FALSE_VALUE:   str += "false"; break;
        case NUMBER_VALUE:  str += std::to_string(value->get_number()); break;
        case STRING_VALUE:  stringify_string(value, str); break;
        case ARRAY_VALUE:   { 
            str += "[ ";
            auto& array = value->get_array();
            for (size_t i = 0; i < array.size(); ++i) {
                if (i) str += ", "; 
                stringify(array[i], str);
            }
            str += " ]";
            break;
        }
        case OBJECT_VALUE:    {
            for (int i = level; i > 0; --i) str += '\t';
            str += "{ \n";
            auto& obj = value->get_object();
            for (auto o = obj.begin(); o != obj.end(); ++o) {
                if (o != obj.begin()) str += ",\n";
                for (int i = level; i >= 0; --i) str += '\t';
                str += ("\"" + o->first + "\" : ");
                stringify_value(o->second, str, level + 1);
            }
            str += '\n';
            for (int i = level; i > 0; --i) str += '\t';
            str += "}";
            break;
        }
        default: assert(0 && "Invalid type");
    }
}

inline void Json::stringify(Value::ValuePtr& value, std::string& str) {
    stringify_value(value, str, 0);
}