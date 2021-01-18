#ifndef __YDSVALUE_H__
#define __YDSVALUE_H__

#include <string.h>
#include <stdlib.h>
#include <assert.h>

/**
 * 数据类型
*/
typedef enum {
    YDS_NULL,
    YDS_TRUE,
    YDS_FALSE,
    YDS_NUMBER,
    YDS_STRING,
    YDS_ARRAY,
    YDS_OBJECT
} yds_type;


/**
 * 保存数据的结构体
*/
class YdsValue {
public:
    YdsValue() : type_(YDS_NULL) {}
    ~YdsValue() { if (type_ == YDS_STRING && s_.s) free(s_.s); }

    yds_type get_type() const { return type_; }
    void set_type(yds_type type) { type_ = type; }

    bool get_boolean() const { assert(type_ == YDS_TRUE || type_ == YDS_FALSE); return type_ == YDS_TRUE; }
    void set_boolean(bool value) { type_ = value ? YDS_TRUE : YDS_FALSE; }

    double get_number() const { assert(type_ == YDS_NUMBER); return num_; }
    void set_number(double number) { if (type_ == YDS_STRING && s_.s) free(s_.s); num_ = number; type_ = YDS_NUMBER; }

    const char* get_string() const { assert(type_ == YDS_STRING); return s_.s; }
    size_t get_string_len() const { assert(type_ == YDS_STRING); return s_.len; }
    void set_string(const char* s, size_t len) { 
        assert(s || len == 0);
        if (type_ == YDS_STRING && s_.s) free(s_.s);
        s_.s = static_cast<char *>(malloc(len + 1));
        memcpy(s_.s, s, len); s_.len = len; 
        s_.s[len] = '\0';
        s_.len = len;
        type_ = YDS_STRING;
    }

private:
    /*使用联合体节省内存*/
    union {
        double num_;
        struct { char* s; size_t len; } s_;
    };
    yds_type type_;
};

#endif // !__YDSVALUE_H__