#pragma once

#include <iostream>
//#include <iterator>

template<typename T>
class Array {
public:
    //using iterator = T*;
    //using const_iterator = const T*;
    //using iterator_category = std::random_access_iterator_tag;


    Array(std::ostream& ostream): Ostream_(ostream), capacity_(2), size_(0) {
        data_ = new T[capacity_];
    }

    Array(const Array& array): Ostream_(array.Ostream_), capacity_(array.capacity_), size_(array.size_){
        if (capacity_ == 0){
            data_ = nullptr;
        } else{
            data_ = new T[capacity_];
            for (size_t i = 0; i < size_; ++i) {
                data_[i] = array.data_[i];
            }
        }
    }

    Array(size_t size, std::ostream& ostream = std::cout, T defaultValue = T{}): Ostream_(ostream), capacity_(2*size), size_(size) {
        if (capacity_ > 0) {
            data_ = new T[capacity_];
            for (size_t i = 0; i < size_; ++i) {
                data_[i] = defaultValue;
            }
        }
    }

    ~Array() {
        delete[] data_;
        data_ = nullptr;
    }

    size_t Size() const {
        return size_;
    }
    size_t Capacity() const {
        return capacity_;
    }

    void Reserve(size_t newCapacity) {
        if (newCapacity > capacity_) {
            T* new_data = new T[newCapacity];
            for (size_t i = 0; i < size_; ++i) {
                new_data[i] = data_[i];
            }
            delete[] data_;
            data_ = new_data;
            capacity_ = newCapacity;
        }
    }
    void Resize(size_t newSize) {
        this->Reserve(newSize);
        size_ = newSize;
    }

    void PushBack(T value = 0) {
        if (size_ == capacity_) {
            capacity_ *= 2;
            if (capacity_ == 0) capacity_ = 2;
            T* newData = new T[capacity_];
            for (size_t i = 0; i < size_; ++i) {
                newData[i] = data_[i];
            }
            delete[] data_;
            data_ = newData;
        }
        data_[size_] = value; //perhaps size_++
        ++size_;
    }

    void PopBack() {
        if (size_ > 0) {
            --size_;
        }
    }
    const T& operator[](const size_t i) const {
        if (i >= size_) throw std::out_of_range("Array index out of range");
        return data_[i];
    }

    T& operator[](const size_t i) {
        if (i >= size_) throw std::out_of_range("Array index out of range");
        return data_[i];
    }

    explicit operator bool() const {
        return (size_ > 0);
    }

    bool operator<(const Array& it) const {
        for (size_t i = 0; i < std::min(size_, it.size_); ++i) {
            if (data_[i] < it.data_[i]) {
                return true;
            } else if (data_[i] > it.data_[i]) {
                return false;
            }
        }
        return size_ < it.size_;
    }

    bool operator>(const Array& it) const {
        return it < *this;
    }
    bool operator!=(const Array& it) const {
        return (it > *this || *this > it);
    }
    bool operator==(const Array& it) const {
         return !(*this != it);
    }
    bool operator<=(const Array& it) const {
        return (*this < it || *this == it);
    }
    bool operator>=(const Array& it) const {
        return (*this > it || *this == it);
    }

    Array& operator<<(const T value) {
        this->PushBack(value);
        return *this;
    }

    Array& operator<<(const Array& it) {
        size_t new_size = size_ + it.size_;
        size_t new_capacity = capacity_ + it.capacity_;
        if (new_capacity == 0) {
            capacity_ = new_capacity;
            size_ = new_size;
            data_ = nullptr;
            return *this;
        }
        int *new_data = new T[new_capacity];
        for (size_t i = 0; i < size_; ++i) {
            new_data[i] = data_[i];
        }
        for (size_t i = 0; i < it.size_; ++i) {
            new_data[size_ + i] = it.data_[i];
        }
        delete[] data_;
        data_ = new_data;
        size_ = new_size;
        capacity_ = new_capacity;

        return *this;
    }

    bool Insert(size_t pos, const T& value) {
        if (pos > size_) {
            return false;
        }
        if (size_ == capacity_) {
            Reserve(capacity_ == 0 ? 2 : capacity_ * 2);
        }
        for (size_t i = size_; i > pos; --i) {
            data_[i] = data_[i - 1];
        }
        data_[pos] = value;
        ++size_;
        return true;
    }
    bool Erase(size_t pos) {
        if (pos >= size_) {
            return false;
        }

        for (int i = (int)(pos); i < (int)(size_) - 1; ++i) {
            data_[i] = data_[i + 1];
        }

        --size_;
        return true;
    }
    T* begin() const{
        return data_;
    }
    T* end() const{
        return data_ + size_;
    }

private:
    std::ostream& Ostream_;
    size_t capacity_;
    size_t size_;
    T *data_ = nullptr;
};

template<typename T>
std::ostream& operator <<(std::ostream& ostream, const Array<T>& array) {
    ostream << "Result Array's capacity is " << array.capacity_ << " and size is " << array.size_;
    if (array.size_ != 0) {
        ostream << ", elements are: " << array.data_[0];
            for (size_t i = 1; i < array.size_; ++i) {
                ostream << ", " << array.data_[i];
            }
        }
        return ostream;
}