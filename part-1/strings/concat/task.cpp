#include "task.h"

char* concat(const char* lhs, const char* rhs) {
    int size1 = 0;
    while (lhs[size1] != '\0') {
        ++size1;
    }
    int size2 = 0;
    while (rhs[size2] != '\0') {
        ++size2;
    }
    char* res = new char[size1 + size2 + 1];
    for (int i = 0; lhs[i] != '\0'; ++i) {
        res[i] = lhs[i];
    }
    for (int j = 0; rhs[j] != '\0'; ++j) {
        res[size1+j] = rhs[j];
    }
    res[size1 + size2] = '\0';
    return res;
}
