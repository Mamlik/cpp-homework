#pragma once
#include <vector>
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <iterator>

template <typename T>
class FlattenedVector {
public:
    class Iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        Iterator(const FlattenedVector* parent, size_t index)
            : parent_(parent), index_(index) {}
        
        reference operator*() const {
            auto [outer, inner] = parent_->find_elem(index_);
            return (*parent_->data_)[outer][inner];
        }
        reference operator[](difference_type i) const{
            return *(*this + i);
        }
        Iterator& operator++() { // префиксный инкремент
            ++index_;
            return *this;
        }
        Iterator operator++(int) { // постфиксный инкремент
            Iterator tmp = *this;
            ++index_; // !!!!!!!!
            return tmp;
        }
        Iterator& operator--() {
                --index_;
                return *this;
        }

        Iterator operator--(int) {
            Iterator tmp = *this;
            --index_;
            return tmp;
        }
        Iterator operator+(difference_type n) const {
            return Iterator(parent_, index_ + n);
        }
        friend Iterator operator+(difference_type n, const Iterator& it) {
            return Iterator(it.parent_, it.index_ + n);
        }
        Iterator operator-(difference_type n) const {
            return Iterator(parent_, index_ - n);
        }
        friend Iterator operator-(difference_type n, const Iterator& it) {
            return Iterator(it.parent_, n - it.index_);
        }
        Iterator& operator+=(difference_type n) {
            index_ += n;
            return *this;
        }
        Iterator& operator-=(difference_type n) {
            index_ -= n;
            return *this;
        }
        difference_type operator-(const Iterator& other) const {
            return index_ - other.index_;
        }
        //difference_type operator+(const Iterator& other) const {  //???
        //    return index_ + other.index_; // ?????
        //} // ?????
        bool operator==(const Iterator& other) const {
            return (index_ == other.index_);
        }
        bool operator!=(const Iterator& other) const {
            return (index_ != other.index_);
        }
        bool operator<(const Iterator& other) const {
            return index_ < other.index_;
        }

        bool operator>(const Iterator& other) const {
            return index_ > other.index_;
        }

        bool operator<=(const Iterator& other) const {
            return index_ <= other.index_;
        }

        bool operator>=(const Iterator& other) const {
            return index_ >= other.index_;
        }

    private:
        const FlattenedVector* parent_;
        size_t index_;
    };
    FlattenedVector(std::vector<std::vector<T>>& data) : data_(&data){

        prefix_.resize(data.size() + 1, 0);
        for (size_t i = 0; i < data.size(); ++i) {
            prefix_[i + 1] = prefix_[i] + data[i].size();
        }
    }
    Iterator begin() {
        return Iterator(this, 0);
    }
     Iterator end() const {
        return Iterator(this, prefix_.back());
    }

private:
    std::vector<size_t> prefix_;
    std::vector<std::vector<T>>* data_ = nullptr;

    std::pair<size_t, size_t> find_elem(size_t index) const{
        auto it = std::upper_bound(prefix_.begin(), prefix_.end(), index);
        size_t outor = std::distance(prefix_.begin(), it) - 1; //!!!!!!!!!!!!!!!!!!!!! perhaps -1
        size_t inner = index - prefix_[outor];
        return {outor, inner};
    }
};
