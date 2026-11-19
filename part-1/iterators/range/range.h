#include <iostream>

class Range {
public:
    class Iterator {
    public:
        Iterator(int64_t current, int64_t step) {
            current_ = current;
            step_ = step;
        }
        int64_t operator*() {
            return current_;
        }
        Iterator& operator++() { //!!!!!!!!!!!!!!!!!!
            current_ += step_;
            return *this;
        }
        Iterator operator++(int) { // постфиксный инкремент
            Iterator temp = *this;
            current_ += step_;
            return temp;
        }
        bool operator==(const Iterator& other) const{
            return (current_ == other.current_);
        }
        bool operator!=(const Iterator& other) const {
            return !(*this == other);
        }
    private:
        int64_t current_;
        int64_t step_;
    };
    
    Range(int64_t n){
        start_ = 0;
        end_ = n;
        step_ = 1;
    }
    Range(int64_t start, int64_t end) {
        start_ = start;
        end_ = end;
        if (start_ <= end_) {
            step_ = 1;
        } else {
            step_ = -1;
        }
    }
    Range(int64_t start, int64_t end, int64_t step) {
        start_ = start;
        step_ = step;
        if (step>0) {
            end_ = start_ + (end - start_ + step_ - 1) / step_ * step_;
        } else {
            end_ = start_ + (end - start_ + step_ + 1) / step_ * step_;
        }
    }
    ~Range() {
        start_ = 0;
        end_ = 0;
        step_ = 1;
    }
    Iterator begin() const{
        return Iterator(start_, step_);
    }
    Iterator end() const{
        return Iterator(end_, step_);
    }
    int64_t Size() const{
    if (step_ == 0) {
        throw std::invalid_argument("Step cannot be zero");
    }
    
    if ((end_ > start_ && step_ > 0) || (end_ < start_ && step_ < 0)) {
        return (end_ - start_ + step_ - (step_ > 0 ? 1 : -1)) / step_;
    } else {
        return 0;
    }
}

private:
    int64_t start_;
    int64_t end_;
    int64_t step_;
};