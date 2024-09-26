#pragma once

#include <span>
#include <utility>

namespace rage {

// !how to iterator: https://internalpointers.com/post/writing-custom-iterators-modern-cpp

// this is both a Matrix and MatrixView Iterator
template <typename T>
class MatrixIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = std::pair<std::size_t, std::span<T>>; // row index and row
        using pointer           = value_type*;
        using reference         = value_type&;
    
    public:
        explicit MatrixIterator(T* start, std::size_t cols_count, std::size_t real_col_count)
            :   row_{0, {start, cols_count}},
                real_col_count_{real_col_count}
        {}

        // copy assignment because of operator++(int)
        MatrixIterator operator=(const MatrixIterator& mi) {
            return {mi.row_, mi.real_col_count_};
        }

        reference operator*() {
            return row_;
        }

        pointer operator->() {
            return &row_;
        }

        MatrixIterator& operator++() {
            row_ = value_type{row_.first + 1, {&row_.second.front() + real_col_count_, row_.second.size()}};
            return *this;
        }

        // postfix
        value_type operator++(int) {
            auto cpy{*this};
            ++(*this);
            return cpy;
        }
        
        friend inline bool operator==(const MatrixIterator& a, const MatrixIterator& b) {
            return &a.row_.second.front() == &b.row_.second.front();
        }

        friend inline bool operator!=(const MatrixIterator& a, const MatrixIterator& b) {
            return &a.row_.second.front() != &b.row_.second.front();
        }

    private:
        value_type row_;
        const std::size_t real_col_count_;
};

} // namespace rage