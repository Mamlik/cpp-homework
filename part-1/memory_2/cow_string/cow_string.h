#include <cstring>

using namespace std;

struct State {
    char* data;
    size_t size;
    size_t capacity;
    int ref_count;

    State(size_t cap) : data(new char[cap]), size(0), capacity(cap), ref_count(1) {
        data[0] = '\0';
    }
    State(const char* str, size_t sz) : data(new char[sz+1]), size(sz), capacity(sz+1), ref_count(1) {
        memcpy(data, str, sz);
        data[sz] = '\0';
    }
    ~State() {//because of memleak
        delete[] data;
    }
    void plus_ref() {
        ++ref_count;
    }
    void minus_ref() {
        --ref_count;
        if (ref_count == 0) {
            delete this;
        }
    }

    State* copy_del() {
        if (ref_count == 1) {
            return this;
        } else {
            --ref_count;
            State* new_state = new State(data, size);
            return new_state;
        }
    }

    void add_memory(size_t new_cap) {
    if (new_cap > capacity) {
        char* new_data = new char[new_cap];
        memcpy(new_data, data, size);
        delete[] data;
        data = new_data;
        capacity = new_cap;
    }
}
};

class CowString {
public:
    // constrcutors
    CowString() : state(new State(2)) {}
    CowString(const CowString& other) : state(other.state) {
        state->plus_ref();
    }
    // copy operator=      //todo
    ~CowString() {
        state->minus_ref();
    }
    CowString& operator=(const CowString& other) {
        if (this != &other) {
            state->minus_ref();
            state = other.state;
            state->plus_ref();
        }
        return *this;
    }

    const char& At(size_t index) const {
        return state->data[index];
    }
    char& operator[](size_t index) {
        state = state->copy_del();
        return state->data[index];
    }

    const char& Back() const {
        return state->data[state->size - 1];
    }

    void PushBack(char c) {
    state = state->copy_del();
    if (state->size + 1 >= state->capacity) { 
        state->add_memory(state->capacity * 2);
    }
    state->data[state->size++] = c; 
    state->data[state->size] = '\0';
}

    size_t Size() const {
        return state->size;
    }
    size_t Capacity() const {
        return state->capacity;
    }

    void Reserve(size_t capacity) {
        state = state->copy_del();
        state->add_memory(capacity);
    }
    void Resize(size_t size) {
        state = state->copy_del();
        if (size > state->capacity) {
            state->add_memory(size + 1);
        }
        if (size > state->size) {
            memset(state->data + state->size, '0', size - state->size);
        }
        state->size = size;
        state->data[state->size] = '\0';
    }
private:
    State* state;
};
