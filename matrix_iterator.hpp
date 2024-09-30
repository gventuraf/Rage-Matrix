#pragma once

#include <span>
#include <functional>
#include <ranges>
#include <concepts>
#include <type_traits>
#include <optional>

namespace internal_impl {
    struct NoType {};
    
    template <typename T>
    using DefaultMorph = std::function<NoType(T)>;

    template<typename Morph, typename T>
    concept MorphConcept = std::is_invocable_v<Morph, T>;
}

namespace rage {

// this is both a Matrix and MatrixView Iterator
template <typename T, typename Morph = internal_impl::DefaultMorph<T>>
requires internal_impl::MorphConcept<Morph, T>
class MatrixIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = std::conditional_t< std::is_same_v<Morph, internal_impl::DefaultMorph<T>>,
                                                    std::span<T>,
                                                    std::ranges::transform_view<std::span<T>, Morph>>;
        using pointer           = value_type*;
        using reference         = value_type&;
    
    public:
        explicit MatrixIterator(T* start, std::size_t cols_count, std::size_t real_col_count, std::optional<Morph> morph = std::nullopt)
            :   row_{start, cols_count},
                real_col_count_{real_col_count}
        {
            if constexpr(!std::is_same_v<Morph, internal_impl::DefaultMorph<T>>) {
                morph_ = morph.value();
                morphed_row_ = row_ | std::views::transform(morph_);
            }
        }
        explicit MatrixIterator(){} //! removing this breaks std::ranges::range<Matrix<T>>, aka breaks everything

        reference operator*() const { //! needs to be const
            if constexpr (std::is_same_v<Morph, internal_impl::DefaultMorph<T>>)
                return row_;
            else
                return morphed_row_;
        }

        pointer operator->() {
            if constexpr (std::is_same_v<Morph, internal_impl::DefaultMorph<T>>)
                return &row_;
            else
                return &morphed_row_;
        }

        MatrixIterator& operator++() {
            row_ = std::span<T>{&row_.front() + real_col_count_, row_.size()};
            if constexpr(!std::is_same_v<Morph, internal_impl::DefaultMorph<T>>)
                morphed_row_ = row_ | std::views::transform(morph_);
            return *this;
        }

        // postfix
        MatrixIterator operator++(int) {
            auto cpy{*this};
            ++(*this);
            return cpy;
        }

    private:
        mutable std::span<T> row_;
        std::size_t real_col_count_;
        Morph morph_;
        mutable value_type morphed_row_;

    private:
        template <typename A, typename B, typename C, typename D> friend bool operator==(const MatrixIterator<A, B>& a, const MatrixIterator<C, D>& b);
        template <typename A, typename B, typename C, typename D> friend bool operator!=(const MatrixIterator<A, B>& a, const MatrixIterator<C, D>& b);
};

template <typename A, typename B, typename C, typename D>
inline bool operator==(const MatrixIterator<A, B>& a, const MatrixIterator<C, D>& b) {
    return &a.row_.front() == &b.row_.front();
}

template <typename A, typename B, typename C, typename D>
inline bool operator!=(const MatrixIterator<A, B>& a, const MatrixIterator<C, D>& b) {
    return &a.row_.front() != &b.row_.front();
}

} // namespace rage