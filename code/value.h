#ifndef __VALUE_H__
#define __VALUE_H__

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <assert.h>
/**
 * 数据类型
*/
typedef enum {
    NULL_VALUE,
    TRUE_VALUE,
    FALSE_VALUE,
    NUMBER_VALUE,
    STRING_VALUE,
    ARRAY_VALUE,
    OBJECT_VALUE,
} value_type;

class Value {
public:
    typedef std::shared_ptr<Value> ValuePtr;
    Value() : type_(NULL_VALUE) {}
    ~Value() {}

    value_type get_type() const { return type_; }
    void set_type(value_type type) { type_ = type; }//此调用不会销毁存储的数据
    void set_null() { clear(); type_ = NULL_VALUE; }

     void set_boolean(bool value) { clear(); type_ = value ? TRUE_VALUE : FALSE_VALUE; }
    bool get_boolean() const { assert(type_==TRUE_VALUE || type_==FALSE_VALUE); return type_ == TRUE_VALUE; }
   
    void set_number(double value) { clear(); num_ = value; type_ = NUMBER_VALUE; }
    double get_number() const { assert(type_ == NUMBER_VALUE); return num_; }

    void set_string(const char *value) { clear(); str_ = value; type_ = STRING_VALUE; }
    std::string& get_string() { assert(type_ == STRING_VALUE); return str_; }

    void set_array(std::vector<ValuePtr>& values) { clear(); array_ = values; type_ = ARRAY_VALUE; }
    std::vector<ValuePtr>& get_array() { assert(type_ == ARRAY_VALUE); return array_; }

    void set_object(std::unordered_map<std::string, ValuePtr>& values) { clear(); obj_ = values; type_ = OBJECT_VALUE; }
    std::unordered_map<std::string, ValuePtr>& get_object() { assert(type_ == OBJECT_VALUE); return obj_; }

private:    
    void clear(){ str_.clear(); array_.clear(); obj_.clear(); type_ = NULL_VALUE; }

private:
    double num_;
    std::string str_;
    std::vector<ValuePtr> array_;
    std::unordered_map<std::string, ValuePtr> obj_;
    value_type type_;
};

#endif // !__VALUE_H__