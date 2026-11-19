#include "task.h"

bool is_palindrome(const std::string& s) {
    std::string s1;
    for (char c: s) {
        if (c >= 'A' && c <= 'Z') {
            c = c + 32;
            s1 += c;
        }
        else if (c >= 'a' && c <= 'z') {
            s1 += c;
        }
    }
    auto begin = s1.begin();
    auto end = --s1.end();
    bool b = true;
    while (begin < end) {
        if (*begin != *end) {
            b = false;
            break;
        }
        ++begin;
        --end;
    }
    return b;
}
