// ************************************************************************** //
//           Advanced Algorithmic Differentiation SS23 - Tutorial 5           //
//                                                                            //
//                        Linear Algebra & Vector mode                        //
//                                                                            //
// Copyright (C) 2023 Software and Tools for Computational Engineering (STCE) //
//               RWTH Aachen University - www.stce.rwth-aachen.de             //
// ************************************************************************** //

#ifndef AD_ET_PREACCUMULATION_HPP_
#define AD_ET_PREACCUMULATION_HPP_

#include "et/expression.hpp"
#include "vector.hpp"

// Include C++ standard header
#include <tuple>

namespace ad {
namespace et {

// ******************** Preaccumulation for expressions ********************* //

/******************************************************************************
 * @brief Evaluates the primal expression for given values of all program
 *        variables on the right-hand side of an assignment; compiler generates
 *        code for its efficient evaluation.
 *
 * Invokes callable OP::compute_value on the tuple of subexpressions provided as
 * second input (single expression for OP=Sin, two expressions for Op=Mul).
 *
 * @tparam OP The operation type (e.g. Sin, Mul, etc.).
 * @tparam ARGS The types of the subexpressions.
 * @param[in] expr The expression we want to evaluate.
 * @returns The result of the primal expression.
 ******************************************************************************/
template<typename OP, typename... ARGS>
decltype(auto) ComputeValue(const expression<OP, ARGS...>& expr) {

   return std::apply(OP::compute_value, expr.get_subexpressions());
}

/******************************************************************************
 * @brief Evaluates gradient and primal expression for given values of all
 *        program variables on the right-hand side of an assignment;
 *        compiler generates code for their efficient evaluation.
 *
 * Invokes callable OP::compute_gradient on the tuple of subexpressions
 * provided as second input (single expression for OP=Sin, two expressions
 * for Op=Mul). The partial derivative is appended to the tuple of
 * subexpressions.
 *
 * @tparam T The passive type.
 * @tparam PARTIAL_T The type of the derivative component.
 * @tparam OP The operation type (e.g. Sin, Mul, etc.).
 * @tparam ARGS The types of the subexpressions.
 * @param[in] expr The expression we want to evaluate.
 * @param[in] d_lhs_d_this The partial derivative of the left-hand side w.r.t.
 *                         the result of this operation.
 * @param[inout] der Reference to the local derivative of the left-hand side.
 * @returns The result of the primal expression.
 ******************************************************************************/
template<typename T, typename PARTIAL_T, typename OP, typename... ARGS>
decltype(auto) ComputeGradient(
     const expression<OP, ARGS...>& expr, T&& d_lhs_d_this, PARTIAL_T& der) {

   return std::apply(
        OP::compute_gradient, std::tuple_cat(
                                   expr.get_subexpressions(),
                                   std::forward_as_tuple(d_lhs_d_this, der)));
}

// ***************** Preaccumulation for passive variables ****************** //

/******************************************************************************
 * @brief Returns the primal value (noop).
 * @tparam T The passive type.
 * @param[in] primal The primal variable.
 * @returns The primal value.
 ******************************************************************************/
template<typename T>
T ComputeValue(const T& primal) {

   return primal;
}

/******************************************************************************
 * @brief Returns the primal value (noop).
 * @tparam T The passive type.
 * @param[in] primal The primal variable.
 * @returns The primal value.
 ******************************************************************************/
template<typename T, typename PARTIAL_T>
T ComputeGradient(const T& primal, T&&, PARTIAL_T&) {

   return primal;
}

}  // namespace et
}  // namespace ad

// ***************** Preaccumulation for adjoint variables ****************** //

// Needed here for adjoint
#include "et/adjoint.hpp"

namespace ad {
namespace et {

/******************************************************************************
 * @brief Returns primal value of an adjoint variable.
 * @tparam T The passive type.
 * @param[in] progvar The adjoint variable.
 * @returns The primal value of the adjoint variable.
 ******************************************************************************/
template<typename T>
T ComputeValue(const adjoint<T>& progvar) {

   return progvar.value;
}

/******************************************************************************
 * @brief Registers program variable as argument of current assignment
 *        together with the corresponding gradient entry.
 * @tparam T The passive type.
 * @param[in] progvar The adjoint variable.
 * @param[in] d_lhs_d_this The partial derivative of the left-hand side w.r.t.
 *                         the adjoint variable.
 * @returns The primal value of the adjoint variable.
 ******************************************************************************/
template<typename T, typename PARTIAL_T>
T ComputeGradient(const adjoint<T>& progvar, const T d_lhs_d_this, PARTIAL_T&) {

   progvar.record_arg(d_lhs_d_this);
   return progvar.value;
}

}  // namespace et
}  // namespace ad

// ***************** Preaccumulation for tangent variables ****************** //

// Needed here for tangent
#include "et/tangent.hpp"

namespace ad {
namespace et {

/******************************************************************************
 * @brief Returns primal value of an tangent variable.
 * @tparam T The passive type.
 * @param[in] progvar The tangent variable.
 * @returns The primal value of the tangent variable.
 ******************************************************************************/
template<typename T, size_t VEC_SIZE>
T ComputeValue(const tangent<T, VEC_SIZE>& progvar) {

   return progvar.value;
}

/******************************************************************************
 * @brief Increments the derivative of the lhs.
 * @tparam T The passive type.
 * @param[in] progvar The tangent variable.
 * @param[in] d_lhs_d_this The partial derivative of the left-hand side w.r.t.
 *                         the tangent variable.
 * @param[inout] der Reference to the local derivative of the left-hand side.
 * @returns The primal value of the tangent variable.
 ******************************************************************************/
template<typename T, size_t VEC_SIZE>
T ComputeGradient(
     const tangent<T, VEC_SIZE>& progvar, const T d_lhs_d_this,
     derivative_vector_t<T, VEC_SIZE>& der) {

   der += progvar.derivative.cwiseProduct(
        derivative_vector_t<T, VEC_SIZE>(d_lhs_d_this));
   return progvar.value;
}

}  // namespace et
}  // namespace ad

// ************************************************************************** //

#endif  // AD_ET_PREACCUMULATION_HPP_
