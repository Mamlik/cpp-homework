#include <cstdlib>
#include <iterator>
#include <cstdint>

/*
 * Нужно написать функцию, которая принимает на вход диапазон, применяет к каждому элементу данную операцию и затем складывает результат применения операции в новый диапазон
 * Входной диапазон задан итераторами [firstIn; lastIn)
 * Выходной диапазон начинается с firstOut и имеет такую же длину как входной диапазон
 * Операция является функцией с одним аргументом (унарная функция), возвращающая результат такого типа, который можно положить в OutputIt
 */

template<class InputIt, class OutputIt, class UnaryOperation>
void Transform(InputIt firstIn, InputIt lastIn, OutputIt firstOut, UnaryOperation op) {
    while (firstIn != lastIn) {
        *firstOut = op(*firstIn);
        ++firstIn;
        ++firstOut;
    }
}

/*
 * Нужно написать функцию, которая принимает на вход диапазон и переставляет элементы в нем таким образом, чтобы элементы,
 * которые удовлетворяют условию p, находились ближе к началу диапазона, чем остальные элементы.
 * Входной диапазон задан итераторами [first; last)
 * p является функцией с одним аргументом (унарная функция), возвращающая результат типа bool
 */

template<class BidirIt, class UnaryPredicate>
void Partition(BidirIt first, BidirIt last, UnaryPredicate p) {
    if (first == last) return;
    --last;
    while (true) {
        while (first != last && p(*first)) {
            ++first;
        }
        while (first != last && !p(*last)) {
            --last;
        }
        if (first >= last) {
            break;
        }
        std::ranges::iter_swap(first, last);
        ++first;
        --last;
    }
}

/*
 * Нужно написать функцию, которая принимает на вход два отстотированных диапазона и объединяет их в новый отсортированный диапазон, содержащий все элементы обоих входных диапазонов.
 */
template<class InputIt1, class InputIt2, class OutputIt>
void Merge(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, OutputIt firstOut) {
    while (first1 != last1 && first2 != last2) {
        if (*first1 < *first2) {
            *firstOut = *first1;
            ++first1;
        } else {
            *firstOut = *first2;
            ++first2;
        }
        ++firstOut;
    }
    //add ostatok
    while (first1 != last1) {
        *firstOut = *first1;
        ++first1;
        ++firstOut;
    }
    while (first2 != last2) {
        *firstOut = *first2;
        ++first2;
        ++firstOut;
    }
}


/*
 * Напишите класс "диапазон чисел Фибоначчи"
 * Экземпляр класса представляет из себя диапазон от первого до N-го числа Фибоначчи (1, 2, 3, 5, 8, 13 и т.д.)
 * С помощью функций begin и end можно получить итераторы и пробежать по диапазону или передать их в STL-алгоритмы
 */
class FibonacciRange {
public:

    class Iterator {
        friend class FibonacciRange;
    public:
        using value_type = uint64_t;
        using difference_type = ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::input_iterator_tag;
        Iterator(const FibonacciRange* parent, size_t index) : 
        parent_(parent), index_(index)
        {}

        value_type operator *() const {
            if (index_ == 0) return 1;
            if (index_ == 1) return 2;
            uint64_t prev = 1, cur = 2;
            for (uint64_t i = 2; i <= index_; ++i) {
                uint64_t next = cur + prev;
                prev = cur;
                cur = next;
            }
            return cur;
        }

        Iterator& operator ++() {
            ++index_;
            return *this;
        }

        Iterator operator ++(int) {
            Iterator tmp = *this;
            ++index_;
            return tmp;
        }

        bool operator ==(const Iterator& rhs) const {
            return (index_ == rhs.index_); 
        }

        bool operator <(const Iterator& rhs) const {
            return (index_ < rhs.index_);  
        }
    private:
        const FibonacciRange* parent_;
        size_t index_;
    };

    FibonacciRange(size_t amount) : amount_(amount) {}

    Iterator begin() const {
        return Iterator(this, 0);
    }

    Iterator end() const {
        return Iterator(this, amount_); // perhaps amount_ + 1
    }

    size_t size() const {
        return amount_;
    }
private:
    uint64_t amount_;
};
