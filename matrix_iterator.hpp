#pragma once

#include <span>

namespace rage {

// !how to iterator: https://internalpointers.com/post/writing-custom-iterators-modern-cpp

// this is both a Matrix and MatrixView Iterator
template <typename T>
class MatrixIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = std::span<T>;
        using pointer           = value_type*;
        using reference         = value_type&;
    
    public:
        explicit MatrixIterator(T* start, std::size_t cols_count, std::size_t real_col_count)
            :   row_{start, cols_count},
                real_col_count_{real_col_count}
        {}

        explicit MatrixIterator(){} //! removing this breaks std::ranges::range<Matrix<T>>

        reference operator*() const { //! need to be const
            return row_;
        }

        pointer operator->() {
            return &row_;
        }

        MatrixIterator& operator++() {
            row_ = value_type{&row_.front() + real_col_count_, row_.size()};
            return *this;
        }

        // postfix
        MatrixIterator operator++(int) {
            auto cpy{*this};
            ++(*this);
            return cpy;
        }
        
        friend inline bool operator==(const MatrixIterator& a, const MatrixIterator& b) {
            return &a.row_.front() == &b.row_.front();
        }

        friend inline bool operator!=(const MatrixIterator& a, const MatrixIterator& b) {
            return &a.row_.front() != &b.row_.front();
        }

    private:
        mutable value_type row_;
        std::size_t real_col_count_;
};

} // namespace rage