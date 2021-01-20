#ifndef __YDSVALUE_H__
#define __YDSVALUE_H__

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>

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
    ~YdsValue() { destroy(); }
    void init() { type_ = YDS_NULL; }

    yds_type get_type() const { return type_; }
    void set_type(yds_type type) { destroy(); type_ = type; }

    bool get_boolean() const { 
        assert(type_ == YDS_TRUE || type_ == YDS_FALSE); 
        return type_ == YDS_TRUE; 
    }
    void set_boolean(bool value) { destroy(); type_ = value ? YDS_TRUE : YDS_FALSE; }

    double get_number() const { assert(type_ == YDS_NUMBER); return num_; }
    void set_number(double number) { destroy(); num_ = number; type_ = YDS_NUMBER; }

    const char* get_string() const { assert(type_ == YDS_STRING); return s_.s; }
    size_t get_string_len() const { assert(type_ == YDS_STRING); return s_.len; }
    void set_string(const char* s, size_t len) { 
        assert(s || len == 0);
        destroy();
        s_.s = static_cast<char *>(malloc(len + 1));
        memcpy(s_.s, s, len); s_.len = len; 
        s_.s[len] = '\0';
        s_.len = len;
        type_ = YDS_STRING;
    }

    void set_array(char* a, size_t size) {
        destroy();
        if (size) {
            a_.e = static_cast<YdsValue *>(malloc(size * sizeof(YdsValue)));
            memcpy(a_.e, a, size * sizeof(YdsValue));
        }
        else a_.e = nullptr;
        
        a_.size = size;
        type_ = YDS_ARRAY;
    }
    //void set_array_size(size_t size) {}
    YdsValue* get_array_element(size_t index) const { assert(type_ == YDS_ARRAY); return &a_.e[index]; }
    size_t get_array_size() const { assert(type_ == YDS_ARRAY); return a_.size; }

    void set_object(char* o, size_t len, size_t size) { 
        destroy();
        if (size) {
            o_.m = static_cast<YdsMember *>(malloc(len));
            memcpy(o_.m, o, size * sizeof(YdsValue));
        }
        else o_.m = nullptr;

        o_.size = size;
        type_ = YDS_OBJECT;
    }
    const char* get_object_key(size_t index) const { assert(type_ == YDS_OBJECT); return o_.m[index].key; }
    size_t get_object_key_len(size_t index) const { assert(type_ == YDS_OBJECT); return o_.m[index].key_len; }
    YdsValue* get_object_value(size_t index) const { assert(type_ == YDS_OBJECT); return o_.m[index].v; }
    size_t get_object_size() const { assert(type_ == YDS_OBJECT); return o_.size; }

    void destroy() {
        switch (type_) {
            case YDS_STRING:
                free(s_.s);
                break;
            case YDS_ARRAY:
                for (int i = 0; i < a_.size; ++i) {
                    //destroy(&a_.e[i]);
                    a_.e[i].destroy();
                }
                //free(&a_);
                free(a_.e);
                break;
            default: 
                break;
        }
        type_ = YDS_NULL;
    }

private:
    struct YdsMember;
    /*使用联合体节省内存*/
    union {
        double num_;/*数字*/
        struct { char* s; size_t len; } s_;/*字符串*/
        struct { YdsValue* e; size_t size; } a_; /*数组*/
        struct { YdsMember* m; size_t size; }o_;

    };
    yds_type type_;
};

struct YdsMember {
    char* key;
    size_t key_len;
    YdsValue v;//?
};

#endif // !__YDSVALUE_H__