#pragma once

#include "matrix_iterator.hpp"
#include "col.hpp"

#include <iostream> //!debug

#include <array>
#include <vector>
#include <initializer_list>
#include <cassert>
#include <algorithm>
#include <ranges>
#include <span>
#include <concepts>
#include <functional>

//* concepts examples: https://itnext.io/c-20-concepts-complete-guide-42c9e009c6bf
//* e.g std::common_type_t<const double, const int> == double

namespace rage
{
    template <typename T, typename Morph = internal_impl::NoMorphFunction<T>>
    class MatrixView;

} // namespace rage

template <typename T, typename M> //!debug
void PrintMatrix(const rage::MatrixView<T, M>& mat) {
    for (const auto& row : mat) {
        for (const auto& elem : row) {
            std::cout << elem << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "************" <<std::endl;
}

namespace {

template <typename T, typename V>
concept Addable = requires(T t, V v) {
    t + v;
};

//* std::convertible_to<T, W> // <From T, To W>

template <typename T, typename V>
concept Multipliable = requires(T t, V v) {
    t * v;
};

/*
template <typename OuterRange>
concept RangeOfRanges = requires(OuterRange r) {
    std::ranges::range<OuterRange>;
    std::ranges::range<std::ranges::range_value_t<OuterRange>>;
};
*/

} // namespace

namespace rage {

//! ***
//! ***
//! *** Matrix
//! ***

//template <typename T, std::size_t RowsCountSize = 0, std::size_t ColsCountSize = 0>
template <typename T>
class Matrix
{
public:
    explicit Matrix(std::size_t rows, std::size_t cols)
        :   rows_count_{rows},
            cols_count_{cols},
            data_{new T[rows * cols]},
            view_{View_()}
    {}

    //TODO change this to take in a range<range<T>>
    explicit Matrix(std::vector<std::vector<T>>&& data)
        :   rows_count_{data.size()},
            cols_count_{data.size()},
            data_{new T[rows_count_ * cols_count_]},
            view_{View_()}
    {
        for (std::size_t i{0}; i < data.size(); ++i) {
            assert(data[i].size() == data[0].size() && "Rows must all have same length");
            std::move(data[i].begin(), data[i].end(), data_ + i * cols_count_);
        }
    }

    //TODO constructor from MatrixView

    template <typename W>
    requires std::convertible_to<W, T>
    Matrix(const Matrix<W>& m)
        :   rows_count_{m.RowsCount()},
            cols_count_{m.ColsCount()},
            data_{new int[m.Size()]},
            view_{View_()}
    {
        std::copy(m.data_, m.data_ + m.Size(), data_);
    }

    template <typename W>
    requires std::convertible_to<W, T>
    Matrix<T> operator=(const Matrix<W>& m)
    {
        rows_count_ = m.RowsCount();
        cols_count_ = m.ColsCount();
        delete[] data_;
        data_ = new T[m.Size()];
        std::copy(m.data_, m.data_ + m.Size(), data_);
        view_ = View_();
    }

    template <typename W>
    requires std::convertible_to<W, T>
    Matrix(Matrix<T>&& m)
        :   rows_count_{m.RowsCount()},
            cols_count_{m.ColsCount()},
            data_{m.data_},
            view_{View_()}
    {
        m.rows_count_ = 0;
        m.cols_count_ = 0;
        m.data_ = nullptr;
    }

    template <typename W>
    requires std::convertible_to<W, T>
    Matrix<T> operator=(Matrix<T>&& m)
    {
        rows_count_ = m.RowsCount();
        cols_count_ = m.ColsCount();
        delete[] data_;
        data_ = m.data_;
        view_ = View_();
        m.rows_count_ = 0;
        m.cols_count_ = 0;
        m.data_ = nullptr;
    }

    ~Matrix() { delete[] data_; }

public:
    template <typename W>
    requires Addable<T, W> && std::convertible_to<W, T>
    constexpr Matrix& Add(const W& val) { view_.Add(val); return *this; };

    template <typename W>
    requires Addable<T, W> && std::convertible_to<W, T>
    constexpr Matrix<T>& Add(const MatrixView<W>& mv) { view_.Add(mv); return *this; }

    template <typename W>
    requires Addable<T, W> && std::convertible_to<W, T>
    constexpr Matrix<T>& Add(const Matrix<W>& m) { return Add(m.view_); }

    template <typename W>
    requires Addable<T, W> && std::convertible_to<W, T>
    constexpr Matrix& Sub(const W& val) { view_.Sub(val); return *this; };

    template <typename W>
    requires Addable<T, W> && std::convertible_to<W, T>
    constexpr Matrix<T>& Sub(const MatrixView<W>& mv) { view_.Sub(mv); return *this; }

    template <typename W>
    requires Addable<T, W> && std::convertible_to<W, T>
    constexpr Matrix<T>& Sub(const Matrix<W>& m) { return Sub(m.view_); }

//* Views
public:
    constexpr MatrixView<T> View(const std::array<std::size_t, 2>& rows, const std::array<std::size_t, 2>& cols) {
        const auto [rows_count, cols_count]{ViewImpl_(rows, cols)};
        return MatrixView<T>{&At(rows[0], cols[0]), rows_count, cols_count, cols_count_};
    }
    
    constexpr MatrixView<const T> View(const std::array<std::size_t, 2>& rows, const std::array<std::size_t, 2>& cols) const {
        const auto [rows_count, cols_count]{ViewImpl_(rows, cols)};
        return MatrixView<const T>{&At(rows[0], cols[0]), rows_count, cols_count, cols_count_};
    }

    constexpr MatrixView<const T> ConstView(const std::array<std::size_t, 2>& rows, const std::array<std::size_t, 2>& cols) const {
        return View(rows, cols);
    }

    constexpr MatrixView<T>& View() {
        return view_;
    }

    template <typename Morph>
    requires std::is_invocable_v<Morph, T>
    constexpr MatrixView<T, Morph> View(Morph morph) {
        return MatrixView<T, Morph>{&At(0, 0), rows_count_, cols_count_, cols_count_, morph};
    }
    
    constexpr MatrixView<const T> View() const {
        return MatrixView<const T>{&At(0, 0), rows_count_, cols_count_, cols_count_};
    }

    constexpr MatrixView<const T> ConstView() const {
        return View();
    }

    bool ReinterpretDimensions(std::size_t new_row_count, std::size_t new_col_count) {
        if (new_row_count * new_col_count != rows_count_ * cols_count_)
            return false;
        rows_count_ = new_row_count;
        cols_count_ = new_col_count;
        view_ = View_();
        return true;
    }

//* Iterators
public:
    constexpr MatrixIterator<T> begin() {
        return MatrixIterator<T>{data_, cols_count_, cols_count_};
    }
    constexpr MatrixIterator<T> end() {
        return MatrixIterator<T>{data_ + Size(), cols_count_, cols_count_};
    }

    constexpr MatrixIterator<const T> begin() const {
        return MatrixIterator<const T>{data_, cols_count_, cols_count_};
    }
    constexpr MatrixIterator<const T> end() const {
        return MatrixIterator<const T>{data_ + Size(), cols_count_, cols_count_};
    }

    constexpr MatrixIterator<const T> cbegin() const {
        return begin();
    }
    constexpr MatrixIterator<const T> cend() const {
        return end();
    }

//* Access methods
public:
    constexpr inline std::span<T> Data() { return std::span<T>{data_, Size()}; }
    constexpr inline std::span<const T> Data() const { return std::span<const T>{data_, Size()}; }
    
    constexpr inline std::size_t RowsCount() const { return rows_count_; }
    constexpr inline std::size_t ColsCount() const { return cols_count_; }

    constexpr std::span<const T> Row(std::size_t r) const { return std::span<const T>(&At(r, 0), cols_count_); }
    constexpr std::span<T> Row(std::size_t r) { return std::span<T>(&At(r, 0), cols_count_); }
    
    constexpr std::span<const T> operator[](std::size_t r) const { return Row(r); }
    constexpr std::span<T> operator[](std::size_t r) { return Row(r); }

    constexpr Column<const T> Col(std::size_t c) const { return Column<const T>{&At(0, c), cols_count_, rows_count_}; }
    constexpr Column<T> Col(std::size_t c) { return Column<T>{&At(0, c), cols_count_, rows_count_}; }
    
    const T& At(std::size_t r, std::size_t c) const { return data_[r * cols_count_ + c]; }
    T& At(std::size_t r, std::size_t c) { return data_[r * cols_count_ + c]; }

//* Lower level operations
//? public or private?
public:
    std::size_t Size() const { return rows_count_ * cols_count_; }

private:
    //TODO should throw?
    constexpr bool CheckView_(std::size_t rows, std::size_t cols) const {
        return rows <= rows_count_ && cols <= cols_count_;
    }

    constexpr std::tuple<std::size_t, std::size_t> ViewImpl_(const std::array<std::size_t, 2>& rows, const std::array<std::size_t, 2>& cols) const {
        const auto rows_count{rows[1] - rows[0] + 1};
        const auto cols_count{cols[1] - cols[0] + 1};
        CheckView_(rows_count, cols_count); //! ignored return value
        return {rows_count, cols_count};
    }

    constexpr MatrixView<T> View_() {
        return MatrixView<T>{&At(0, 0), rows_count_, cols_count_, cols_count_};
    }

private:
    std::size_t rows_count_;
    std::size_t cols_count_;
    T* data_;
    MatrixView<T> view_;

private:
    template <typename U, typename M> friend class MatrixView;
    template <typename U> friend class Matrix;
};


//! ***
//! ***
//! *** MatrixView
//! ***

template <typename T, typename Morph>
class MatrixView
{
//* Operations
public:
    template <typename W>
    requires Addable<T, W> && std::convertible_to<W, T>
    constexpr MatrixView& Add(const W& val);

    template <typename W>
    requires Addable<T, W> && std::convertible_to<W, T>
    constexpr MatrixView& Add(const MatrixView<W>& mv);

    template <typename W>
    requires Addable<T, W> && std::convertible_to<W, T>
    constexpr MatrixView& Add(const Matrix<W>& m) { return Add(m.view_); }

    template <typename W>
    requires Addable<T, W> && std::convertible_to<W, T>
    constexpr MatrixView& Sub(const W& val) { return Add(-val); }

    template <typename W>
    requires Addable<T, W> && std::convertible_to<W, T>
    constexpr MatrixView& Sub(const MatrixView<W>& mv);

    template <typename W>
    requires Addable<T, W> && std::convertible_to<W, T>
    constexpr MatrixView& Sub(const Matrix<W>& m) { return Sub(m.view_); }

//* Views
public:
    constexpr MatrixView<const T> ConstView(const std::array<std::size_t, 2>& rows, const std::array<std::size_t, 2>& cols) const { //TODO morph_ how?
        const auto [rows_count, cols_count]{ViewImpl_(rows, cols)};
        return MatrixView<const T>{&At(rows[0], cols[0]), rows_count, cols_count, cols_count_};
    }

    constexpr MatrixView<T> View(const std::array<std::size_t, 2>& rows, const std::array<std::size_t, 2>& cols) { //TODO morph_ how?
        const auto [rows_count, cols_count]{ViewImpl_(rows, cols)};
        return MatrixView<T>{&At(rows[0], cols[0]), rows_count, cols_count, cols_count_};
    }

    //Note: ConstView() ? should it exist?

//* Iterators
public:
    constexpr MatrixIterator<T, Morph> begin() {
        return MatrixIterator<T, Morph>{data_start_, cols_count_, real_col_count_, morph_};
    }
    constexpr MatrixIterator<T, Morph> end() {
        return MatrixIterator<T, Morph>{data_start_ + rows_count_ * real_col_count_, 0, 0, morph_};
    }

    constexpr MatrixIterator<const T, Morph> begin() const {
        return cbegin();
    }
    constexpr MatrixIterator<const T, Morph> end() const {
        return cend();
    }

    constexpr MatrixIterator<const T, Morph> cbegin() const {
        return MatrixIterator<const T, Morph>{data_start_, cols_count_, real_col_count_, morph_};
    }
    constexpr MatrixIterator<const T, Morph> cend() const {
        return MatrixIterator<const T, Morph>{data_start_ + rows_count_ * real_col_count_, 0, 0, morph_};
    }

//* Access methods
public:
    constexpr inline std::size_t RowsCount() const { return rows_count_; }
    constexpr inline std::size_t ColsCount() const { return cols_count_; }

    constexpr std::span<const T> Row(std::size_t r) const { return std::span<const T>(&At(r, 0), cols_count_); } //TODO morph_
    constexpr std::span<T> Row(std::size_t r) { return std::span<T>(&At(r, 0), cols_count_); } //TODO morph_

    constexpr std::span<const T> operator[](std::size_t r) const { return Row(r); }
    constexpr std::span<T> operator[](std::size_t r) { return Row(r); }

    constexpr Column<const T> Col(std::size_t c) const { return Column<const T>{&At(0, c), cols_count_, rows_count_}; } //TODO morph_
    constexpr Column<T> Col(std::size_t c) { return Column<T>{&At(0, c), cols_count_, rows_count_}; } //TODO morph_

    /*
    constexpr const T& At(std::size_t r, std::size_t c) const
    {
        const auto& elem{data_start_[r * real_col_count_ + c]};
        if constexpr (std::is_same_v<Morph, std::function<void()>>)
            return elem;
        else
            return morph_(elem);
    }
    */
    
    constexpr const T& At(std::size_t r, std::size_t c) const { return data_start_[r * real_col_count_ + c]; } //TODO morph_
    constexpr T& At(std::size_t r, std::size_t c) { return data_start_[r * real_col_count_ + c]; } //TODO morph_

//* Lower level operations
public:
    constexpr std::size_t Size() const { return rows_count_ * cols_count_; }

private:
    constexpr explicit MatrixView(T* data_start, std::size_t rows_count,
        std::size_t cols_count, std::size_t real_col_count,
        std::optional<Morph> morph = std::nullopt)
        :   data_start_{data_start},
            rows_count_{rows_count},
            cols_count_{cols_count},
            real_col_count_{real_col_count}
    {
        if constexpr (!std::is_same_v<Morph, internal_impl::NoMorphFunction<T>>)
            morph_ = morph.value();
    }

private:
    constexpr bool CheckView_(std::size_t rows, std::size_t cols) const {
        return rows <= rows_count_ && cols <= cols_count_;
    }

    constexpr std::tuple<std::size_t, std::size_t> ViewImpl_(const std::array<std::size_t, 2>& rows, const std::array<std::size_t, 2>& cols) const {
        const auto rows_count{rows[1] - rows[0] + 1};
        const auto cols_count{cols[1] - cols[0] + 1};
        CheckView_(rows_count, cols_count); //! ignored return value
        return {rows_count, cols_count};
    }

private:
    T* data_start_;
    std::size_t rows_count_;
    std::size_t cols_count_;
    std::size_t real_col_count_;
    Morph morph_;

private:
    template <typename U> friend class Matrix;
    template <typename U, typename M> friend class MatrixView;
};

//! ***
//! ***
//! Operators
//! ***

//*
//* Addition
template <typename T, typename W, typename MorphOne, typename MorphTwo, typename R = std::common_type_t<T, W>>
constexpr Matrix<R> operator+(const MatrixView<T, MorphOne>& lhs, const MatrixView<W, MorphTwo>& rhs);

template <typename T, typename W, typename R = std::common_type_t<T, W>>
inline constexpr Matrix<R> operator+(const Matrix<T>& lhs, const Matrix<W>& rhs) { return lhs.View() + rhs.View(); }

template <typename T, typename W, typename Morph, typename R = std::common_type_t<T, W>>
inline constexpr Matrix<R> operator+(const Matrix<T>& lhs, const MatrixView<W, Morph>& rhs) { return lhs.View() + rhs; }

template <typename T, typename W, typename Morph, typename R = std::common_type_t<T, W>>
inline constexpr Matrix<R> operator+(const MatrixView<T, Morph>& lhs, const Matrix<W>& rhs) { return rhs + lhs; }

template <typename T, typename W, typename Morph, typename R = std::common_type_t<T, W>>
constexpr Matrix<R> operator+(const MatrixView<T, Morph>& lhs, const W& val);

template <typename T, typename W, typename Morph, typename R = std::common_type_t<T, W>>
inline constexpr Matrix<R> operator+(const W& val, const MatrixView<T, Morph>& rhs) { return rhs + val; }

template <typename T, typename W, typename R = std::common_type_t<T, W>>
inline constexpr Matrix<R> operator+(const Matrix<T>& lhs, const W& val) { return lhs.View() + val; }

template <typename T, typename W, typename R = std::common_type_t<T, W>>
inline constexpr Matrix<R> operator+(const W& val, const Matrix<T>& rhs) { return rhs.View() + val; }

//*
//* Subtraction
template <typename T, typename W, typename R = std::common_type_t<T, W>>
constexpr Matrix<R> operator-(const MatrixView<T>& lhs, const MatrixView<W>& rhs);

template <typename T, typename W, typename R = std::common_type_t<T, W>>
inline constexpr Matrix<R> operator-(const Matrix<T>& lhs, const Matrix<W>& rhs) { return lhs.View() - rhs.View(); }

template <typename T, typename W, typename Morph, typename R = std::common_type_t<T, W>>
inline constexpr Matrix<R> operator-(const Matrix<T>& lhs, const MatrixView<W, Morph>& rhs) { return lhs.View() - rhs; }

template <typename T, typename W, typename Morph, typename R = std::common_type_t<T, W>>
inline constexpr Matrix<R> operator-(const MatrixView<T, Morph>& lhs, const Matrix<W>& rhs) { return rhs - lhs; }

template <typename T, typename W, typename Morph, typename R = std::common_type_t<T, W>>
constexpr Matrix<R> operator-(const MatrixView<T, Morph>& lhs, const W& val);

template <typename T, typename W, typename Morph, typename R = std::common_type_t<T, W>>
inline constexpr Matrix<R> operator-(const W& val, const MatrixView<T, Morph>& rhs);

template <typename T, typename W, typename R = std::common_type_t<T, W>>
inline constexpr Matrix<R> operator-(const Matrix<T>& lhs, const W& val) { return lhs.View() - val; }

template <typename T, typename W, typename R = std::common_type_t<T, W>>
inline constexpr Matrix<R> operator-(const W& val, const Matrix<T>& rhs) { return val - rhs.View(); }

//*
//* Multiplication
template <typename T, typename W, typename MorphOne, typename MorphTwo, typename R = std::common_type_t<T, W>>
requires Multipliable<T, W>
constexpr Matrix<R> operator*(const MatrixView<T, MorphOne>& lhs, const MatrixView<W, MorphTwo>& rhs);

template <typename T, typename W, typename R = std::common_type_t<T, W>>
requires Multipliable<T, W>
inline constexpr Matrix<R> operator*(const Matrix<T>& lhs, const Matrix<W>& rhs) { return lhs.View() * rhs.View(); }

template <typename T, typename W, typename Morph, typename R = std::common_type_t<T, W>>
requires Multipliable<T, W>
inline constexpr Matrix<R> operator*(const Matrix<T>& lhs, const MatrixView<W, Morph>& rhs) { return lhs.View() * rhs; }

template <typename T, typename W, typename Morph, typename R = std::common_type_t<T, W>>
requires Multipliable<T, W>
inline constexpr Matrix<R> operator*(const MatrixView<T, Morph>& lhs, const Matrix<W>& rhs) { return lhs * rhs.View(); }

template <typename T, typename W, typename Morph, typename R = std::common_type_t<T, W>>
requires Multipliable<T, W>
constexpr Matrix<R> operator*(const MatrixView<T, Morph>& lhs, const W& val);

template <typename T, typename W, typename Morph, typename R = std::common_type_t<T, W>>
requires Multipliable<T, W>
inline constexpr Matrix<R> operator*(const W& val, const MatrixView<T, Morph>& rhs) { return rhs * val; }

template <typename T, typename W, typename R = std::common_type_t<T, W>>
requires Multipliable<T, W>
inline constexpr Matrix<R> operator*(const Matrix<T>& lhs, const W& val) { return lhs.View() * val; }

template <typename T, typename W, typename R = std::common_type_t<T, W>>
requires Multipliable<T, W>
inline constexpr Matrix<R> operator*(const W& val, const Matrix<T>& rhs) { return rhs.View() * val; }

} // namespace rage


//! ***
//! ***
//! Utils Implementation
//! ***

template <typename T, typename W, typename R = std::common_type_t<T, W>>
constexpr R DotProduct(const std::span<T>& v1, const Column<W>& v2)
{
    const auto len{v1.size()};
    R total{};
    for (std::size_t i{0}; i < len; ++i)
        total += v1[i] * v2[i];
    return total;
}

template <typename T, typename W, typename R = std::common_type_t<T, W>>
constexpr R DotProduct(const Column<W>& v1, const std::span<T>& v2)
{
    return DotProduct(v2, v1);
}

//! ***
//! ***
//! Classes Implementation
//! ***

namespace rage{


template <typename T, typename Morph>
template <typename W>
requires Addable<T, W> && std::convertible_to<W, T>
constexpr MatrixView<T, Morph>& MatrixView<T, Morph>::Add(const W& val)
{
    for (auto& row : *this) {
        for (auto& elem : row)
            elem += val;
    }
    
    return *this;
}

template <typename T, typename Morph>
template <typename W>
requires Addable<T, W> && std::convertible_to<W, T>
constexpr MatrixView<T, Morph>& MatrixView<T, Morph>::Add(const MatrixView<W>& rhs)
{
    for (auto [idx, row] : *this | std::ranges::views::enumerate) {
        auto rhs_row{rhs[idx]};
        for (std::size_t i{0}; i < cols_count_; ++i)
            row[i] += rhs_row[i];
    }

    return *this;
}

//! This code "duplication" isn't the best, ideally i'd use the Add to implement Sub
//TODO change once I have custom piping similar to std::ranges::views
template <typename T, typename Morph>
template <typename W>
requires Addable<T, W> && std::convertible_to<W, T>
constexpr MatrixView<T, Morph>& MatrixView<T, Morph>::Sub(const MatrixView<W>& rhs)
{
    //TODO return Add(rhs.View([](const auto& elem){ return -elem; }));
    //*
    for (auto [idx, row] : *this | std::ranges::views::enumerate) {
        auto rhs_row{rhs[idx]};
        for (std::size_t i{0}; i < cols_count_; ++i)
            row[i] -= rhs_row[i];
    }

    return *this;
}

//! ***
//! ***
//! Operators Implementation
//! ***

//*
//* Addition
template<typename T, typename W, typename MorphOne, typename MorphTwo, typename R>
constexpr Matrix<R> operator+(const MatrixView<T, MorphOne>& lhs, const MatrixView<W, MorphTwo>& rhs)
{
    const auto rows_count{lhs.RowsCount()};
    const auto cols_count{lhs.ColsCount()};
    
    Matrix<R> result(rows_count, cols_count);
    
    for (std::size_t r{0}; r < rows_count; ++r) {
        for (std::size_t c{0}; c < cols_count; ++c)
            result[r][c] = lhs[r][c] + rhs[r][c];
    }
    
    return result;
}

template <typename T, typename W, typename Morph, typename R>
constexpr Matrix<R> operator+(const MatrixView<T, Morph>& lhs, const W& val)
{
    const auto rows_count{lhs.RowsCount()};
    const auto cols_count{lhs.ColsCount()};
    
    Matrix<R> result(rows_count, cols_count);
    
    for (std::size_t r{0}; r < rows_count; ++r) {
        for (std::size_t c{0}; c < cols_count; ++c)
            result[r][c] = lhs[r][c] + val;
    }
    
    return result;
}

//*
//* Subtraction
template<typename T, typename W, typename MorphOne, typename MorphTwo,  typename R>
constexpr Matrix<R> operator-(const MatrixView<T, MorphOne>& lhs, const MatrixView<W, MorphTwo>& rhs)
{
    const auto rows_count{lhs.RowsCount()};
    const auto cols_count{lhs.ColsCount()};
    
    Matrix<R> result(rows_count, cols_count);
    
    for (std::size_t r{0}; r < rows_count; ++r) {
        for (std::size_t c{0}; c < cols_count; ++c)
            result[r][c] = lhs[r][c] - rhs[r][c];
    }
    
    return result;
}

template <typename T, typename W, typename Morph, typename R>
constexpr Matrix<R> operator-(const MatrixView<T, Morph>& lhs, const W& val)
{
    const auto rows_count{lhs.RowsCount()};
    const auto cols_count{lhs.ColsCount()};
    
    Matrix<R> result(rows_count, cols_count);
    
    for (std::size_t r{0}; r < rows_count; ++r) {
        for (std::size_t c{0}; c < cols_count; ++c)
            result[r][c] = lhs[r][c] - val;
    }
    
    return result;
}

// code duplication of function above
template <typename T, typename W, typename Morph, typename R>
inline constexpr Matrix<R> operator-(const W& val, const MatrixView<T, Morph>& rhs)
{
    const auto rows_count{rhs.RowsCount()};
    const auto cols_count{rhs.ColsCount()};
    
    Matrix<R> result(rows_count, cols_count);
    
    for (std::size_t r{0}; r < rows_count; ++r) {
        for (std::size_t c{0}; c < cols_count; ++c)
            result[r][c] = val - rhs[r][c];
    }
    
    return result;
}

//*
//* Multiplication

template <typename T, typename W, typename MorphOne, typename MorphTwo, typename R>
requires Multipliable<T, W>
constexpr Matrix<R> operator*(const MatrixView<T, MorphOne>& lhs, const MatrixView<W, MorphTwo>& rhs)
{
    const auto rows_count{lhs.RowsCount()};
    const auto cols_count{rhs.ColsCount()};
    
    Matrix<R> result(rows_count, cols_count);

    for (std::size_t rhs_col_i{0}; rhs_col_i < cols_count; ++rhs_col_i) {
        const auto rhs_col{rhs.Col(rhs_col_i)};
        for (const auto& [lhs_row_i, lhs_row] : lhs | std::ranges::views::enumerate) {
            result[lhs_row_i][rhs_col_i] = DotProduct(lhs_row, rhs_col);
        }
    }

    return result;
}

template <typename T, typename W, typename Morph,  typename R>
requires Multipliable<T, W>
constexpr Matrix<R> operator*(const MatrixView<T, Morph>& lhs, const W& val)
{
    const auto rows_count{lhs.RowsCount()};
    const auto cols_count{lhs.ColsCount()};
    
    Matrix<R> result(rows_count, cols_count);
    
    for (std::size_t r{0}; r < rows_count; ++r) {
        for (std::size_t c{0}; c < cols_count; ++c)
            result[r][c] = lhs[r][c] * val;
    }
    
    return result;
}

} // namespace rage













//* Notes:

//TODO annotate with noexcept

//TODO make it lazy

//* util functions like: B | rage::views::negative or rge::views::negative(B)

//TODO look at the assembly code generated by Add(const MatrixView<W>& mv) and Add(const Matrix<W>& m)