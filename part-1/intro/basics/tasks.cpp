#include "tasks.h"

#include <cmath>

int NOuter::NInner::DoSomething(int lhs, int rhs) {
    return lhs+rhs;
}

int NOuter::DoSomething(int lhs, int rhs) {
    return lhs - rhs;
}

int NOverload::ProcessTwoArgs(int lhs, int rhs) {
    return lhs+rhs;
}

char NOverload::ProcessTwoArgs(char lhs, char rhs) {
    if (lhs>rhs) {
        return lhs;
    } else{
        return rhs;
    }
}

int NOverload::ProcessTwoArgs(int lhs, char rhs) {
    return lhs - rhs;
}

unsigned int NOverflow::WithOverflow(int lhs, int rhs) {
    return (unsigned int) lhs+rhs;
}

uint64_t NOverflow::WithOverflow(int64_t lhs, int64_t rhs) {
    return (uint64_t) lhs-rhs;
}

int NLoop::SumInRange(const int lhs, const int rhs) {
    int sum = 0;
    for (int i = lhs; i<rhs; ++i) {
        sum+=i;
    }
    return sum;
}

int NLoop::CountFixedBitsInRange(const int from, const int to, const int bitsCnt) {
    int ans = 0;
    for (int i = from; i<to; ++i) {
        int ed = __builtin_popcount(i);
        if (ed == bitsCnt) ans++;
    }
    return ans;
}

double NMath::ComputeMathFormula(const double arg) {
    return std::abs((std::sin(arg) / 2 + std::cos(arg)) * (std::sin(arg) / 2 + std::cos(arg)) + std::tan(arg) * std::atan(arg));
}

bool NMath::IsPositive(int arg) {
    if (arg>0) {
        return true;
    } else {
        return false;
    }
}

int NRecursion::CalculateFibonacci(const int arg) {
    int a = 0, b = 1;
    for(int i=2;i<=arg;i++)
    {
        int c = a + b;
        a = b;
        b = c;
    }
    return b;
}