#pragma once
#include <type_traits>
template <class NumberType> concept Integral = std::is_integral_v<NumberType>;
template <class NumberType> concept FloatingPoint = std::is_floating_point_v<NumberType>;
template <class NumberType> concept Arithmetic = std::is_arithmetic_v<NumberType>;