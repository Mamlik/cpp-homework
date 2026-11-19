#include <iostream>

class Array {
public:
    Array(std::ostream& ostream): Ostream_(ostream), capacity_(2), size_(0) {
        data_ = new int[capacity_];
        Ostream_ << "Constructed. " << *this << '\n';
    }

    Array(const Array& array): Ostream_(array.Ostream_), capacity_(array.capacity_), size_(array.size_){
        if (capacity_ == 0){
            data_ = nullptr;
        } else{
            data_ = new int[capacity_];
            for (size_t i = 0; i < size_; ++i) {
                data_[i] = array.data_[i];
            }
            Ostream_ << "Constructed from another Array. " << *this << '\n';
        }
    }

    Array(size_t size, std::ostream& ostream = std::cout, int defaultValue = 0): Ostream_(ostream), capacity_(2*size), size_(size) {
        if (capacity_ > 0) {
            data_ = new int[capacity_];
            for (size_t i = 0; i < size_; ++i) {
                data_[i] = defaultValue;
            }
        }
         Ostream_ << "Constructed with default. " << *this << '\n';
    }

    ~Array() {
        delete[] data_;
        data_ = nullptr;
        Ostream_ << "Destructed " << size_ << '\n';
    }

    size_t Size() const {
        return size_;
    }
    size_t Capacity() const {
        return capacity_;
    }
    void Reserve(size_t newCapacity) {
        if (newCapacity > capacity_) {
            int* new_data = new int[newCapacity];
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
    void PushBack(int value = 0) {
        if (size_ == capacity_) {
            capacity_ *= 2;
            if (capacity_ == 0) capacity_ = 2;
            int* newData = new int[capacity_];
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

    const int& operator[](const size_t i) const {
        if (i >= size_) throw std::out_of_range("Array index out of range");
        return data_[i];
    }
    int& operator[](const size_t i) {
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
    Array& operator<<(const int value) {
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
        int *new_data = new int[new_capacity];
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
    friend std::ostream& operator<<(std::ostream& ostream, const Array& array) {
        ostream << "Result Array's capacity is " << array.capacity_ << " and size is " << array.size_;
        if (array.size_ != 0) {
            ostream << ", elements are: " << array.data_[0];
            for (size_t i = 1; i < array.size_; ++i) {
                ostream << ", " << array.data_[i];
            }
        }
        return ostream;
    }

private:
    std::ostream& Ostream_;
    size_t capacity_;
    size_t size_;
    int *data_ = nullptr;
};

