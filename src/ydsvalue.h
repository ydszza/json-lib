#ifndef __YDSVALUE_H__
#define __YDSVALUE_H__


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
    YdsValue();
    ~YdsValue() = default;

    yds_type get_type() const { return type_; }
    double get_number() const { return num_; }
    
    void set_type(yds_type type) { type_ = type; }
    void set_number(double number) { num_ = number; }

private:
    /*使用联合体节省内存*/
    union {
        double num_;
    };
    yds_type type_;
};

#endif // !__YDSVALUE_H__