#pragma once
#include <iostream>
#include <algorithm>

namespace NPointers {
    void Increment(int* x) {
        (*x)++;
    }
    int Multiply(int x, int y, bool* flag) {
        int z = x*y;
        if (z/x==y) {
            (*flag)=false;
            return z;
        } else{
            (*flag)=true;
            return z;
        }
    }
    long long ScalarProduct(const int* a, const int* b, int size_arr) {
        long long ans=0;
        for (long long i = 0; i < size_arr; ++i) {
            ans += *(a+i) * *(b+i);
        }
        return ans;
    }
    long long* MultiplyToLongLong(int a, int b) {
        long long a1 = static_cast<long long>(a);
        long long b1 = static_cast<long long>(b);
        long long* ans = new long long;
        *ans = a1*b1;
        return ans;
    }
    int SizeOfMaximumSquareOfCrosses(const char* a, int h, int w) {
        int ans = 0;
        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < w; ++j) {
                for (int size = 0; size <= std::min(w-1-j, h-1-i); ++size) {
                    bool f = true;
                    for (int i1 = i; i1<= i+size; ++i1) {
                        for (int j1 = j; j1<=j+size; ++j1) {
                            if (*(a + i1 * w + j1) != '+') {
                                f = false;
                                break;
                            }
                        }
                        if (f == false) break;
                    }
                    if (f) ans = std::max(ans, size+1);
                }
            }
        }
        return ans;
    }
    /* To implement:
    SizeOfMaximumSquareOfCrosses
    */
}

namespace NReferences {
    void MultiplyInplace(int& result, int num) {
        result *= num;
    }
    int CompareArraysByAverage(const int* arr1, int size, const int* arr2) {
        int sum1 = 0;
        for (int i = 0; i < size; ++i) {
            sum1 += *(arr1 + i);
        }
        double avg1 = static_cast<double>(sum1) / size;

        int sum2 = 0;
        for (int i = 0; i < 5; ++i) {
            sum2 += *(arr2 + i);
        }
        double avg2 = static_cast<double>(sum2) / 5;

        if (avg1<avg2) return -1;
        if (avg2<avg1) return 1;
        if (avg1==avg2) return 0;
        return 0;
    }
    /* To implement:
    */
}
