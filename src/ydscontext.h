#ifndef __YDSCONTEXT_H__
#define __YDSCONTEXT_H__


class YdsContext {
public:
    YdsContext() = default;
    ~YdsContext() = default;

    void set_context(const char* json) { json_ = json; }
    const char* get_context() const { return json_; }

private:
    const char* json_;
};


#endif // !__YDSCONTEXT_H__