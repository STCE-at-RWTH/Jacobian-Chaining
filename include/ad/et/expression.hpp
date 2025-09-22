// ************************************************************************** //
//           Advanced Algorithmic Differentiation SS23 - Tutorial 5           //
//                                                                            //
//                        Linear Algebra & Vector mode                        //
//                                                                            //
// Copyright (C) 2023 Software and Tools for Computational Engineering (STCE) //
//               RWTH Aachen University - www.stce.rwth-aachen.de             //
// ************************************************************************** //

#ifndef AD_ET_EXPRESSION_HPP_
#define AD_ET_EXPRESSION_HPP_

// Include C++ standard header
#include <tuple>
#include <type_traits>

// ************************** Forward declarations ************************** //

namespace ad {
namespace et {

struct Sin;
struct Cos;
struct Mul;
struct Div;
struct Add;
struct Sub;

}  // namespace et
}  // namespace ad

// *********************** Expression implementation ************************ //

namespace ad {
namespace et {

/******************************************************************************
 * @brief Generic expression class.
 * @tparam OP The operation type (e.g. Sin, Mul, etc.).
 * @tparam ARGS The types of the subexpressions.
 ******************************************************************************/
template<typename OP, typename... ARGS>
class expression {

 public:
   //! Builds new expression over given subexpressions.
   expression(ARGS const&... subexpressions);

   //! Returns subexpressions as a tuple.
   const std::tuple<ARGS...>& get_subexpressions() const;

 protected:
   //! Stores subexpressions (one or two).
   const std::tuple<ARGS...> subexpressions;
};

/******************************************************************************
 * @brief Builds new expression over given subexpressions.
 * @tparam OP The operation type (e.g. Sin, Mul, etc.).
 * @tparam ARGS The types of the subexpressions.
 * @param[in] subexpressions Parameter pack of subexpressions.
 ******************************************************************************/
template<typename OP, typename... ARGS>
expression<OP, ARGS...>::expression(ARGS const&... subexpressions)
   : subexpressions(subexpressions...) {}

/******************************************************************************
 * @brief Returns subexpressions as a tuple.
 * @tparam OP The operation type (e.g. Sin, Mul, etc.).
 * @tparam ARGS The types of the subexpressions.
 * @returns Tuple of subexpressions.
 ******************************************************************************/
template<typename OP, typename... ARGS>
const std::tuple<ARGS...>& expression<OP, ARGS...>::get_subexpressions() const {
   return subexpressions;
}

/******************************************************************************
 * @brief Multiplication operator for two expressions.
 * @tparam T1 Type of the first subexpression.
 * @tparam T2 Type of the second subexpression.
 * @param[in] subexpression1 The multiplier expression.
 * @param[in] subexpression2 The multiplicand expression.
 * @returns New mulplication expression containing the two subexpressions.
 ******************************************************************************/
template<typename T1, typename T2>
expression<Mul, T1, T2> operator*(T1&& subexpression1, T2&& subexpression2) {
   return expression<Mul, T1, T2>(subexpression1, subexpression2);
}

/******************************************************************************
 * @brief Division operator for two expressions.
 * @tparam T1 Type of the first subexpression.
 * @tparam T2 Type of the second subexpression.
 * @param[in] subexpression1 The dividend expression.
 * @param[in] subexpression2 The divisor expression.
 * @returns New division expression containing the two subexpressions.
 ******************************************************************************/
template<typename T1, typename T2>
expression<Div, T1, T2> operator/(T1&& subexpression1, T2&& subexpression2) {
   return expression<Div, T1, T2>(subexpression1, subexpression2);
}

/******************************************************************************
 * @brief Addition operator for two expressions.
 * @tparam T1 Type of the first subexpression.
 * @tparam T2 Type of the second subexpression.
 * @param[in] subexpression1 The augend expression.
 * @param[in] subexpression2 The addend expression.
 * @returns New addition expression containing the two subexpressions.
 ******************************************************************************/
template<typename T1, typename T2>
expression<Add, T1, T2> operator+(T1&& subexpression1, T2&& subexpression2) {
   return expression<Add, T1, T2>(subexpression1, subexpression2);
}

/******************************************************************************
 * @brief Subtraction operator for two expressions.
 * @tparam T1 Type of the first subexpression.
 * @tparam T2 Type of the second subexpression.
 * @param[in] subexpression1 The minuend expression.
 * @param[in] subexpression2 The subtrahend expression.
 * @returns New subtraction expression containing the two subexpressions.
 ******************************************************************************/
template<typename T1, typename T2>
expression<Sub, T1, T2> operator-(T1&& subexpression1, T2&& subexpression2) {
   return expression<Sub, T1, T2>(subexpression1, subexpression2);
}

/******************************************************************************
 * @brief Sine function for an expression.
 * @tparam T Type of the subexpression.
 * @param[in] subexpression The argument expression.
 * @returns New sine expression containing the subexpression.
 ******************************************************************************/
template<typename T>
expression<Sin, T> sin(T&& subexpression) {
   return expression<Sin, T>(subexpression);
}

/******************************************************************************
 * @brief Cosine function for an expression.
 * @tparam T Type of the subexpression.
 * @param[in] subexpression The argument expression.
 * @returns New cosine expression containing the subexpression.
 ******************************************************************************/
template<typename T>
expression<Cos, T> cos(T&& subexpression) {
   return expression<Cos, T>(subexpression);
}

}  // namespace et
}  // namespace ad

// ************************************************************************** //

#endif  // AD_ET_EXPRESSION_HPP_
