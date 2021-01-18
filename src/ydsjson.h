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
    YDS_PARSE_OK = 0,                    /*解析成功*/
    YDS_PARSE_EXPECT_VALUE,              /*全是空白*/
    YDS_PARSE_INVALID_VALUE,             /*无效的值*/
    YDS_PARSE_ROOT_NOT_SINGULAR,         /*解析之后还有未解析完的数据*/
    YDS_PARSE_NUMBER_TOO_BIG,            /*解析的数字数值太大*/
    YDS_PARSE_INVALID_STRING_ESCAPE,     /*字符串解析出错*/
    YDS_PARSE_MISS_QUOTATION_MARK,       /*字符串不完整*/
    YDS_PARSE_INVALID_STRING_CHAR,       /*包含无效的字符*/
    YDS_PARSE_INVALID_UNICODE_HEX,       /*无效的Unicode十六进制码*/
    YDS_PARSE_INVALID_UNICODE_SURROGATE  /*无效的代理码*/
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
    int parse_string();
    const char* parse_hex4(const char* p, unsigned* u);
    void encode_utf8(unsigned u);

private:
    YdsValue* value_;        /*保存解析结果的数据结构*/
    YdsContext context_;    /*解析过程的缓存空间*/
};

#endif // !__YDSJSON_H__