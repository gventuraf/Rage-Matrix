#include "matrix.hpp"
#include <print>

template <typename T, typename M>
void PrintMatrix(const rage::MatrixView<T, M>& mat, std::string_view title = "Matrix");

template <typename T>
void PrintMatrix(const rage::Matrix<T>& mat, std::string_view title = "Matrix");

int main()
{
    rage::Matrix<int> water{{{10, 20, 30}, {40, 50, 60}, {70, 80, 90}}};
    rage::Matrix<int> flame{{{5, 15, 25}, {35, 45, 55}, {65, 75, 85}}};

    static_assert(std::input_or_output_iterator<rage::ColumnIterator<int, decltype([](int i){return i;})>>);

    //* Results from mental math and WolframAlpha, any wrong is WA bug
    rage::Matrix<int> water_plus_flame{{{15, 35, 55}, {75, 95, 115}, {135, 155, 175}}};
    rage::Matrix<int> water_minus_flame{{{5, 5, 5}, {5, 5, 5}, {5, 5, 5}}};
    rage::Matrix<int> water_times_flame{{{2700, 3300, 3900}, {5850, 7350, 8850}, {9000, 11400, 13800}}};
    rage::Matrix<int> flame_plus_5{{{10, 20, 30}, {40, 50, 60}, {70, 80, 90}}};
    rage::Matrix<int> water_minus_5{{{5, 15, 25}, {35, 45, 55}, {65, 75, 85}}};
    rage::Matrix<int> five_minus_water{{{-5, -15, -25}, {-35, -45, -55}, {-65, -75, -85}}};
    rage::Matrix<int> water_times_10{{{100, 200, 300}, {400, 500, 600}, {700, 800, 900}}};
    rage::Matrix<int> water_times_1dot5{{{15, 30, 45}, {60, 75, 90}, {105, 120, 135}}};
    rage::Matrix<int> zeros{{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}};
    //*

    //*
    //* Compare

    {
        rage::Matrix<int> water_copy{{{10, 20, 30}, {40, 50, 60}, {70, 80, 90}}};
        assert(water == water_copy);
        assert(!(water != water_copy));
        assert(water != flame);
    }

    //*
    //* Basic operations: + - *

    assert(water + flame == water_plus_flame);
    assert(water - flame == water_minus_flame);
    assert(water * flame == water_times_flame);
    
    assert(flame + 5 == flame_plus_5);
    assert(5 + flame == flame_plus_5);

    assert(water - 5 == water_minus_5);
    assert(5 - water == five_minus_water); // spooky, treats 5 as a matrix where all elements are 5

    assert(water * 10 == water_times_10);
    assert(1.5 * water == water_times_1dot5);

    { // sike, not yet :(
        //constexpr auto result{water * 1.5};
        //static_assert(result == water_times_1dot5);
    }

    //*
    //* There are also methods for + and -

    assert(water.Add(flame) == water_plus_flame);
    {
        auto& water_ref{water.Sub(flame)}; // return is reference to water
        assert(&water_ref == &water);
    }
    
    assert(flame.Add(10).Add(water).Sub(10) == water_plus_flame); // chains!!!
    flame.Sub(water);

    //*
    //* Views: think of it as a std::span or std::string_view, but for Matrix

    auto water_view{water.View()};
    water_view[0][0] = 42; // this will affect Matrix
    assert(water[0][0] == 42);
    water_view[0][0] = 10; // let's change it back

    {
        auto flame_middle_row{flame.View({1, 1}, {0, 2})}; // type is MatrixView
        rage::Matrix<int> a_matrix{{{35, 45, 55}}};
        assert(flame_middle_row == a_matrix);
    }


    //* Can use the same operations for a view
    assert(water_view + flame == water_plus_flame);
    assert(water_view * 1.5 == water_times_1dot5);
    assert(flame.View() + 5 == flame_plus_5);
    
    // does not matter the operation, argumens go in any order
    // does not matter if it is Matrix or MatrixView first
    assert(water * flame == water_times_flame);
    assert(water * flame.View() == water_times_flame);
    assert(water_view * flame == water_times_flame);
    assert(water.View() * flame.View() == water_times_flame);

    //*
    //* Transform

    assert(water - flame.View([](const int& elem) { return -elem; }) == water_plus_flame);
    
    {
        auto f5 = flame.View([](const auto& elem){ return elem + 5; });
        assert(f5 == water);

        auto minus_water{water.View([](const int& elem) { return -elem; })};
        assert(f5.View([](const int& elem) { return -elem; }) == minus_water);
    }


    std::println("Completed successfully!");
    return 0;
}

//*
//* Print
template <typename T, typename M>
void PrintMatrix(const rage::MatrixView<T, M>& mat, std::string_view title) {
    std::println("*** {} ***", title);
    
    for (const auto& row : mat) {
        for (const auto& elem : row) {
            std::print("{} ", elem);
        }
        std::println();
    }
    
    std::println();
}

template <typename T>
void PrintMatrix(const rage::Matrix<T>& mat, std::string_view title) {
    PrintMatrix(mat.View(), title);
}