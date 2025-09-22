// ************************************************************************** //
//           Advanced Algorithmic Differentiation SS23 - Tutorial 5           //
//                                                                            //
//                        Linear Algebra & Vector mode                        //
//                                                                            //
// Copyright (C) 2023 Software and Tools for Computational Engineering (STCE) //
//               RWTH Aachen University - www.stce.rwth-aachen.de             //
// ************************************************************************** //

#ifndef AD_ET_OPERATIONS_HPP_
#define AD_ET_OPERATIONS_HPP_

#include "et/preaccumulation.hpp"

// Include C++ standard header
#include <cmath>

// ********************* Adjoint operation expressions ********************** //

namespace ad {
namespace et {

/******************************************************************************
 * @brief Active sine operation in form of a struct.
 ******************************************************************************/
struct Sin {

   /****************************************************************************
    * @brief Computation of the value of sine of an expression.
    * @param[in] expr The active input expression.
    * @returns The sine of the evaluated subexpression.
    ****************************************************************************/
   static constexpr auto compute_value = [](auto&& expr) {
      return std::sin(ComputeValue(expr));
   };

   /****************************************************************************
    * @brief Computation of the gradient of sine of an expression.
    * @param[in] expr The active input expression.
    * @param[in] d_lhs_d_this The partial derivative of the left-hand side
    *w.r.t. the result of this operation.
    * @param[inout] der Reference to the local derivative of the left-hand side.
    * @returns The sine of the evaluated subexpression.
    ****************************************************************************/
   static constexpr auto compute_gradient = [](auto&& expr, auto&& d_lhs_d_this,
                                               auto& der) {
      return std::sin(ComputeGradient(
           expr, d_lhs_d_this * std::cos(ComputeValue(expr)), der));
   };
};

/******************************************************************************
 * @brief Active cosine operation in form of a struct.
 ******************************************************************************/
struct Cos {

   /****************************************************************************
    * @brief Computation of the value of cosine of an expression.
    * @param[in] expr The active input expression.
    * @returns The cosine of the evaluated subexpression.
    ****************************************************************************/
   static constexpr auto compute_value = [](auto&& expr) {
      return std::cos(ComputeValue(expr));
   };

   /****************************************************************************
    * @brief Computation of the gradient of cosine of an expression.
    * @param[in] expr The active input expression.
    * @param[in] d_lhs_d_this The partial derivative of the left-hand side
    *w.r.t. the result of this operation.
    * @param[inout] der Reference to the local derivative of the left-hand side.
    * @returns The cosine of the evaluated subexpression.
    ****************************************************************************/
   static constexpr auto compute_gradient = [](auto&& expr, auto&& d_lhs_d_this,
                                               auto& der) {
      return std::cos(ComputeGradient(
           expr, -d_lhs_d_this * std::sin(ComputeValue(expr)), der));
   };
};

/******************************************************************************
 * @brief Active multiplication operation in form of a struct.
 ******************************************************************************/
struct Mul {

   /****************************************************************************
    * @brief Computation of the value of multiplication of two expressions.
    * @param[in] expr1 The multiplier expression.
    * @param[in] expr2 The multiplicand expression.
    * @returns The product of the evaluated subexpressions.
    ****************************************************************************/
   static constexpr auto compute_value = [](auto&& expr1, auto&& expr2) {
      return ComputeValue(expr1) * ComputeValue(expr2);
   };

   /****************************************************************************
    * @brief Computation of the gradient of multiplication of two expressions.
    * @param[in] expr1 The multiplier expression.
    * @param[in] expr2 The multiplicand expression.
    * @param[in] d_lhs_d_this The partial derivative of the left-hand side
    *w.r.t. the result of this operation.
    * @param[inout] der Reference to the local derivative of the left-hand side.
    * @returns The product of the evaluated subexpressions.
    ****************************************************************************/
   static constexpr auto compute_gradient = [](auto&& expr1, auto&& expr2,
                                               auto&& d_lhs_d_this, auto& der) {
      const auto value1 = ComputeValue(expr1);
      const auto value2 = ComputeValue(expr2);
      return ComputeGradient(expr1, d_lhs_d_this * value2, der) *
             ComputeGradient(expr2, d_lhs_d_this * value1, der);
   };
};

/******************************************************************************
 * @brief Active division operation in form of a struct.
 ******************************************************************************/
struct Div {

   /****************************************************************************
    * @brief Computation of the value of division of two expressions.
    * @param[in] expr1 The dividend expression.
    * @param[in] expr2 The divisor expression.
    * @returns The quotient of the evaluated subexpressions.
    ****************************************************************************/
   static constexpr auto compute_value = [](auto&& expr1, auto&& expr2) {
      return ComputeValue(expr1) / ComputeValue(expr2);
   };

   /****************************************************************************
    * @brief Computation of the gradient of division of two expressions.
    * @param[in] expr1 The dividend expression.
    * @param[in] expr2 The divisor expression.
    * @param[in] d_lhs_d_this The partial derivative of the left-hand side
    *w.r.t. the result of this operation.
    * @param[inout] der Reference to the local derivative of the left-hand side.
    * @returns The quotient of the evaluated subexpressions.
    ****************************************************************************/
   static constexpr auto compute_gradient = [](auto&& expr1, auto&& expr2,
                                               auto&& d_lhs_d_this, auto& der) {
      const auto value1 = ComputeValue(expr1);
      const auto value2 = ComputeValue(expr2);
      return ComputeGradient(expr1, d_lhs_d_this / value2, der) /
             ComputeGradient(
                  expr2, -d_lhs_d_this * value1 / (value2 * value2), der);
   };
};

/******************************************************************************
 * @brief Active addition operation in form of a struct.
 ******************************************************************************/
struct Add {

   /****************************************************************************
    * @brief Computation of the value of addition of two expressions.
    * @param[in] expr1 The augend expression.
    * @param[in] expr2 The addend expression.
    * @returns The sum of the evaluated subexpressions.
    ****************************************************************************/
   static constexpr auto compute_value = [](auto&& expr1, auto&& expr2) {
      return ComputeValue(expr1) + ComputeValue(expr2);
   };

   /****************************************************************************
    * @brief Computation of the gradient of addition of two expressions.
    * @param[in] expr1 The augend expression.
    * @param[in] expr2 The addend expression.
    * @param[in] d_lhs_d_this The partial derivative of the left-hand side
    *w.r.t. the result of this operation.
    * @param[inout] der Reference to the local derivative of the left-hand side.
    * @returns The sum of the evaluated subexpressions.
    ****************************************************************************/
   static constexpr auto compute_gradient = [](auto&& expr1, auto&& expr2,
                                               auto&& d_lhs_d_this, auto& der) {
      return ComputeGradient(expr1, d_lhs_d_this, der) +
             ComputeGradient(expr2, d_lhs_d_this, der);
   };
};

/******************************************************************************
 * @brief Active subtraction operation in form of a struct.
 ******************************************************************************/
struct Sub {

   /****************************************************************************
    * @brief Computation of the value of subtraction of two expressions.
    * @param[in] expr1 The minuend expression.
    * @param[in] expr2 The subtrahend expression.
    * @returns The difference of the evaluated subexpressions.
    ****************************************************************************/
   static constexpr auto compute_value = [](auto&& expr1, auto&& expr2) {
      return ComputeValue(expr1) - ComputeValue(expr2);
   };

   /****************************************************************************
    * @brief Computation of the gradient of subtraction of two expressions.
    * @param[in] expr1 The minuend expression.
    * @param[in] expr2 The subtrahend expression.
    * @param[in] d_lhs_d_this The partial derivative of the left-hand side
    *w.r.t. the result of this operation.
    * @param[inout] der Reference to the local derivative of the left-hand side.
    * @returns The difference of the evaluated subexpressions.
    ****************************************************************************/
   static constexpr auto compute_gradient = [](auto&& expr1, auto&& expr2,
                                               auto&& d_lhs_d_this, auto& der) {
      return ComputeGradient(expr1, d_lhs_d_this, der) -
             ComputeGradient(expr2, -d_lhs_d_this, der);
   };
};

}  // namespace et
}  // namespace ad

// ************************************************************************** //

#endif  // AD_ET_OPERATIONS_HPP_
