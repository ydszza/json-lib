#ifndef __YDSCONTEXT_H__
#define __YDSCONTEXT_H__

#include <stddef.h>
#include <assert.h>
#include <stdlib.h>

#define YDS_PARSE_STATIC_INIT_SIZE  256

class YdsContext {
public:
    YdsContext() : json_(nullptr), stack_(nullptr), top_(0), size_(0) {}
    ~YdsContext() {
        if (stack_) free(stack_);
    }

    void set_context(const char* json) { json_ = json; }
    const char* get_context() const { return json_; }
    void set_top(size_t top) { top_ = top; }
    size_t get_top() const { return top_; }

    void* buff_push(size_t size) {
        void* ret;
        assert(size > 0);
        if (top_+size >= size_) {
            if (size_ == 0) 
                size_ = YDS_PARSE_STATIC_INIT_SIZE;
            while(top_+size >= size_) 
                size_ += (size_ >> 1);
            stack_ = static_cast<char *>(realloc(stack_, size_));
        }
        ret = stack_ + top_;
        top_ += size;
        return ret;
    }
    void* buff_pop(size_t size) {
        assert(top_ >= size);
        return stack_ + (top_ -= size);
    }

private:
    const char* json_;
    char* stack_;
    size_t top_, size_;
};


#endif // !__YDSCONTEXT_H__