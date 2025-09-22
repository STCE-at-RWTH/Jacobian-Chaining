// ************************************************************************** //
//           Advanced Algorithmic Differentiation SS23 - Tutorial 5           //
//                                                                            //
//                        Linear Algebra & Vector mode                        //
//                                                                            //
// Copyright (C) 2023 Software and Tools for Computational Engineering (STCE) //
//               RWTH Aachen University - www.stce.rwth-aachen.de             //
// ************************************************************************** //

#ifndef AD_NOET_TANGENT_HPP_
#define AD_NOET_TANGENT_HPP_

// Include C++ standard header
#include <cmath>
#include <iostream>

#include "ad/vector.hpp"

// ************************ First order tangent type ************************ //

namespace ad {
namespace noet {

/******************************************************************************
 * @brief First order tangent type with T precision base type.
 * @tparam T Passive type.
 * @tparam VEC_SIZE Vector size (defaults to 1).
 ******************************************************************************/
template<typename T, size_t VEC_SIZE = 1>
struct tangent {

   //! Primal function value.
   T value {0.0};

   //! Directional derivative.
   vector_t<T, VEC_SIZE> derivative {0.0};

   // Default constructors and destructors.
   tangent() = default;
   ~tangent() = default;

   //! Constructor with value component.
   explicit tangent(T);
};

/******************************************************************************
 * @brief Constructor with value component
 * @param[in] val The passive value.
 ******************************************************************************/
template<typename T, size_t VEC_SIZE>
tangent<T, VEC_SIZE>::tangent(const T val) : value(val) {}

/******************************************************************************
 * @brief Multiplication operator for tangent types (active * active).
 * @param[in] arg1 The active multiplier.
 * @param[in] arg2 The active multiplicand.
 * @returns A tangent variable containing the result and derivative of the
 *          active multiplication.
 ******************************************************************************/
template<typename T, size_t VEC_SIZE>
tangent<T, VEC_SIZE> operator*(
     const tangent<T, VEC_SIZE>& arg1, const tangent<T, VEC_SIZE>& arg2) {
   tangent<T, VEC_SIZE> res;
   res.value = arg1.value * arg2.value;
   res.derivative = arg1.value * arg2.derivative + arg1.derivative * arg2.value;
   return res;
}

/******************************************************************************
 * @brief Multiplication operator for tangent types (passive * active).
 * @param[in] arg1 The passive multiplier.
 * @param[in] arg2 The active multiplicand.
 * @returns A tangent variable containing the result and derivative of the
 *          active multiplication.
 ******************************************************************************/
template<typename T, size_t VEC_SIZE>
tangent<T, VEC_SIZE> operator*(const T arg1, const tangent<T, VEC_SIZE>& arg2) {
   tangent<T, VEC_SIZE> res;
   res.value = arg1 * arg2.value;
   res.derivative = arg1 * arg2.derivative;
   return res;
}

/******************************************************************************
 * @brief Multiplication operator for tangent types (active * active).
 * @param[in] arg1 The active multiplier.
 * @param[in] arg2 The passive multiplicand.
 * @returns A tangent variable containing the result and derivative of the
 *          active multiplication.
 ******************************************************************************/
template<typename T, size_t VEC_SIZE>
tangent<T, VEC_SIZE> operator*(
     const tangent<T, VEC_SIZE>& arg1, const T& arg2) {
   tangent<T, VEC_SIZE> res;
   res.value = arg1.value * arg2;
   res.derivative = arg1.derivative * arg2;
   return res;
}

/******************************************************************************
 * @brief Division operator for tangent types (active / passive).
 * @param[in] arg1 The active dividend.
 * @param[in] arg2 The passive divisor.
 * @returns A tangent variable containing the result and derivative of the
 *          active division.
 ******************************************************************************/
template<typename T, size_t VEC_SIZE>
tangent<T, VEC_SIZE> operator/(const tangent<T, VEC_SIZE>& arg1, const T arg2) {
   tangent<T, VEC_SIZE> res;
   res.value = arg1.value / arg2;
   res.derivative = arg1.derivative / arg2;
   return res;
}

/******************************************************************************
 * @brief Addition operator for tangent types (active * active).
 * @param[in] arg1 The active augent.
 * @param[in] arg2 The active addend.
 * @returns A tangent variable containing the result and derivative of the
 *          active addition.
 ******************************************************************************/
template<typename T, size_t VEC_SIZE>
tangent<T, VEC_SIZE> operator+(
     const tangent<T, VEC_SIZE>& arg1, const tangent<T, VEC_SIZE>& arg2) {
   tangent<T, VEC_SIZE> res;
   res.value = arg1.value + arg2.value;
   res.derivative = arg1.derivative + arg2.derivative;
   return res;
}

/******************************************************************************
 * @brief Subtraction operator for tangent types (active * passive).
 * @param[in] arg1 The active minuend.
 * @param[in] arg2 The active subtrahend.
 * @returns A tangent variable containing the result and derivative of the
 *          active subtraction.
 ******************************************************************************/
template<typename T, size_t VEC_SIZE>
tangent<T, VEC_SIZE> operator-(const tangent<T, VEC_SIZE>& arg1, const T arg2) {
   tangent<T, VEC_SIZE> res;
   res.value = arg1.value - arg2;
   res.derivative = arg1.derivative;
   return res;
}

/******************************************************************************
 * @brief Sine function for tangent types.
 * @param[in] arg The active input.
 * @returns A tangent variable containing the result and derivative of the
 *          sine function.
 ******************************************************************************/
template<typename T, size_t VEC_SIZE>
tangent<T, VEC_SIZE> sin(const tangent<T, VEC_SIZE>& arg) {
   tangent<T, VEC_SIZE> res;
   res.value = std::sin(arg.value);
   res.derivative = std::cos(arg.value) * arg.derivative;
   return res;
}

/******************************************************************************
 * @brief Cosine function for tangent types.
 * @param[in] arg The active input.
 * @returns A tangent variable containing the result and derivative of the
 *          cosine function.
 ******************************************************************************/
template<typename T, size_t VEC_SIZE>
tangent<T, VEC_SIZE> cos(const tangent<T, VEC_SIZE>& arg) {
   tangent<T, VEC_SIZE> res;
   res.value = std::cos(arg.value);
   res.derivative = -std::sin(arg.value) * arg.derivative;
   return res;
}

/******************************************************************************
 * @brief Exponential function for tangent types.
 * @param[in] arg The active input.
 * @returns A tangent variable containing the result and derivative of the
 *          exponential function.
 ******************************************************************************/
template<typename T, size_t VEC_SIZE>
tangent<T, VEC_SIZE> exp(const tangent<T, VEC_SIZE>& arg) {
   tangent<T, VEC_SIZE> res;
   res.value = std::exp(arg.value);
   res.derivative = res.value * arg.derivative;
   return res;
}

/******************************************************************************
 * @brief Power function for tangent types (active / passive).
 * @param[in] arg1 The active base.
 * @param[in] arg2 The passive exponent.
 * @returns A tangent variable containing the result and derivative of the
 *          power function.
 ******************************************************************************/
template<typename T, size_t VEC_SIZE>
tangent<T, VEC_SIZE> pow(const tangent<T, VEC_SIZE>& arg1, const T arg2) {
   tangent<T, VEC_SIZE> res;
   res.value = std::pow(arg1.value, arg2);
   res.derivative = arg2 * std::pow(arg1.value, arg2 - 1) * arg1.derivative;
   return res;
}

/******************************************************************************
 * @brief Output stream to print a tangent variable.
 * @param[inout] out Reference to the output stream.
 * @param[in] arg The tangent variable.
 * @returns The reference to the output stream.
 ******************************************************************************/
template<typename T, size_t VEC_SIZE>
std::ostream& operator<<(std::ostream& out, const tangent<T, VEC_SIZE>& arg) {
   out << "{value: " << arg.value << ", derivative: " << arg.derivative << "}";
   return out;
}

}  // namespace noet
}  // namespace ad

// ************************************************************************** //

#endif  // AD_NOET_TANGENT_HPP_
