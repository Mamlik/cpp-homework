#include "task.h"

void ReverseList(std::list<int>& l) {
    auto begin = l.begin();
    auto end = --l.end();
    while (begin != end) {
        std::swap(*begin, *end);
        ++begin;
        --end;
    }
}
