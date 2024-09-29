#pragma once

#include <iterator>

template <typename T>
class ColumnIterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = T;
    using pointer           = value_type*;
    using reference         = value_type&;

    reference operator*() const {
        return *start_;
    }

    pointer operator->() {
        return start_;
    }

    ColumnIterator& operator++() {
        start_ += next_col_offset_;
        return *this;
    }

    // postfix
    value_type operator++(int) {
        auto cpy{*this};
        ++(*this);
        return cpy;
    }
    
    friend inline bool operator==(const ColumnIterator& a, const ColumnIterator& b) {
        return a.start_ == b.start_;
    }

    friend inline bool operator!=(const ColumnIterator& a, const ColumnIterator& b) {
        return a.start_ != b.start_;
    }
private:
    T* start_;
    std::size_t next_col_offset_;
};

template <typename T>
class Column
{
    using S = std::remove_const_t<T>;
public:
    explicit Column(T* start, std::size_t next_col_offset, std::size_t cols_count)
        :   start_{start},
            next_col_offset_{next_col_offset},
            size_{cols_count}
    {}
    
    T& operator[](std::size_t idx) { return start_[idx * next_col_offset_]; }
    const T& operator[](std::size_t idx) const { return start_[idx * next_col_offset_]; }
    std::size_t Size() const { return size_; }

    //* Iterators
    ColumnIterator<T> begin() { return ColumnIterator<T>{start_, next_col_offset_, size_}; }
    ColumnIterator<T> end() { return ColumnIterator<T>{start_ + next_col_offset_ * size_, 0, 0}; }
    
    ColumnIterator<const S> begin() const { return ColumnIterator<const S>{start_, next_col_offset_, size_}; }
    ColumnIterator<const S> end() const { return ColumnIterator<const S>{start_ + next_col_offset_ * size_, 0, 0}; }
    
    ColumnIterator<const S> cbegin() const { return begin(); }
    ColumnIterator<const S> cend() const { return end(); }

private:
    T* start_;
    std::size_t next_col_offset_;
    std::size_t size_;
};