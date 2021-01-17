#ifndef __YDSJSON_H__
#define __YDSJSON_H__

#include <stddef.h>     /*size_t*/
#include <string>
#include <assert.h>     /*assert()*/
#include <errno.h>      /*errno*/
#include <math.h>       /*HUGE_VAL*/
#include "ydsvalue.h"
#include "ydscontext.h"
/**
 * 定义解析结果返回值
*/
enum {
    YDS_PARSE_OK = 0,               /*解析成功*/
    YDS_PARSE_EXPECT_VALUE,         /*全是空白*/
    YDS_PARSE_INVALID_VALUE,        /*无效的值*/
    YDS_PARSE_ROOT_NOT_SINGULAR,    /*解析之后还有未解析完的数据*/
    YDS_PARSE_NUMBER_TOO_BIG
};

class YdsJson {
public:
    int parse(YdsValue* value, const char* json);
    //int parse(const std::string& json);

private:
    int parse_value();
    void parse_whitespace();
    int parse_literial(const char* literal, yds_type type);     /*解析字面量*/
    int parse_number();

private:
    YdsValue* value_;        /*保存解析结果的数据结构*/
    YdsContext context_;    /*解析过程的缓存空间*/
};

#endif // !__YDSJSON_H__