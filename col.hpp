#pragma once

#include <iterator>
#include "matrix_iterator.hpp"

namespace rage {

// can rename to NonContiguousIterator
template <typename T, typename Morph>
class ColumnIterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = T;
    using pointer           = value_type*;
    using reference         = value_type&;

    using RealValueType = std::conditional_t<std::is_same_v<Morph, internal_impl::DefaultMorph<T>>,
                                             T,
                                             std::invoke_result_t<Morph, T>>;

    explicit ColumnIterator(T* start, std::size_t next_col_offset, std::optional<Morph> morph)
    :   start_{start},
        next_col_offset_{next_col_offset}
    {
        if constexpr(!std::is_same_v<Morph, internal_impl::DefaultMorph<T>>)
            morph_ = morph.value();
    }

    explicit ColumnIterator(){} //! removing this breaks std::ranges::range<Matrix<T>>, aka breaks everything

    reference operator*() const {
        if constexpr (std::is_same_v<Morph, internal_impl::DefaultMorph<T>>) {
            return *start_;
        } else {
            morphed_val_ = morph_(*start_);
            return morphed_val_.value();
        }
    }

    pointer operator->() {
        if constexpr (std::is_same_v<Morph, internal_impl::DefaultMorph<T>>) {
            return start_;
        } else {
            morphed_val_ = morph_(*start_);
            return &(morphed_val_.value());
        }
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
    Morph morph_;
    std::optional<RealValueType> morphed_val_;
};


template <typename T, typename Morph = internal_impl::DefaultMorph<T>>
class Column
{
    //using S = std::remove_const_t<T>;
    using RealValueType = std::conditional_t<std::is_same_v<Morph, internal_impl::DefaultMorph<T>>,
                                             T,
                                             std::invoke_result_t<Morph, T>>;

public:
    constexpr explicit Column(T* start, std::size_t next_col_offset, std::size_t cols_count,
                    std::optional<Morph> morph = std::nullopt)
        :   start_{start},
            next_col_offset_{next_col_offset},
            size_{cols_count}
    {
        if constexpr (!std::is_same_v<Morph, internal_impl::DefaultMorph<T>>)
            morph_ = morph.value();
    }
    
    constexpr std::conditional_t<std::is_same_v<Morph, internal_impl::DefaultMorph<T>>, T&, RealValueType>
    operator[](std::size_t idx) {
        if constexpr (std::is_same_v<Morph, internal_impl::DefaultMorph<T>>)
            return start_[idx * next_col_offset_];
        else
            return morph_(start_[idx * next_col_offset_]);
    }
    
    constexpr std::conditional_t<std::is_same_v<Morph, internal_impl::DefaultMorph<T>>, const T&, RealValueType>
    operator[](std::size_t idx) const {
        if constexpr (std::is_same_v<Morph, internal_impl::DefaultMorph<T>>)
            return start_[idx * next_col_offset_];
        else
            return morph_(start_[idx * next_col_offset_]);
    }
    
    //TODO since this is a range, to keep the api the same, maybe change to size() with lowercase s
    constexpr std::size_t Size() const { return size_; }

    //* Iterators
    ColumnIterator<T, Morph> begin() { return ColumnIterator<T, Morph>{start_, next_col_offset_, morph_}; }
    ColumnIterator<T, Morph> end() { return ColumnIterator<T, Morph>{start_ + next_col_offset_ * size_, 0, morph_}; }
    
    ColumnIterator<const T, Morph> begin() const { return ColumnIterator<const T, Morph>{start_, next_col_offset_, morph_}; }
    ColumnIterator<const T, Morph> end() const { return ColumnIterator<const T, Morph>{start_ + next_col_offset_ * size_, 0, morph_}; }
    
    ColumnIterator<const T, Morph> cbegin() const { return begin(); }
    ColumnIterator<const T, Morph> cend() const { return end(); }

private:
    T* start_;
    std::size_t next_col_offset_;
    std::size_t size_;
    Morph morph_;
};

} // namespace rage