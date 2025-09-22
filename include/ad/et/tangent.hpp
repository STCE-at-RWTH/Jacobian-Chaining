// ************************************************************************** //
//           Advanced Algorithmic Differentiation SS23 - Tutorial 5           //
//                                                                            //
//                        Linear Algebra & Vector mode                        //
//                                                                            //
// Copyright (C) 2023 Software and Tools for Computational Engineering (STCE) //
//               RWTH Aachen University - www.stce.rwth-aachen.de             //
// ************************************************************************** //

#ifndef AD_ET_TANGENT_HPP_
#define AD_ET_TANGENT_HPP_

// Include C++ standard header
#include <cmath>

#include "et/expression.hpp"
#include "vector.hpp"

// ************************ First order tangent type ************************ //

namespace ad {
namespace et {

/******************************************************************************
 * @brief First order tangent type with expression template support.
 * @tparam T Passive type.
 * @tparam VEC_SIZE Vector size (defaults to 1).
 ******************************************************************************/
template<typename T, size_t VEC_SIZE = 1>
struct tangent {

   // Default constructors and destructors.
   tangent() = default;
   ~tangent() = default;

   //! Constructor with value component.
   explicit tangent(T);

   //! Copy constructor.
   tangent(const tangent<T, VEC_SIZE>& other);

   //! Assignment of value to tangent program variable.
   tangent<T, VEC_SIZE>& operator=(const T& value);

   //! Assignment of expression to tangent program variable.
   template<typename OP, typename... ARGS>
   tangent<T, VEC_SIZE>& operator=(const et::expression<OP, ARGS...>& rhs);

   //! Primal function value.
   T value = 0.0;

   //! Directional derivative.
   derivative_vector_t<T, VEC_SIZE> derivative =
        derivative_vector_t<T, VEC_SIZE>(0.0);
};

}  // namespace et
}  // namespace ad

// Needed here for et::ComputeGradient
#include "et/preaccumulation.hpp"

namespace ad {
namespace et {

/******************************************************************************
 * @brief Constructor with value component.
 * @param[in] val The passive value.
 ******************************************************************************/
template<typename T, size_t VEC_SIZE>
tangent<T, VEC_SIZE>::tangent(const T val) : value(val) {}

/******************************************************************************
 * @brief Copy constructor.
 * @param[in] other The other tangent variable.
 ******************************************************************************/
template<typename T, size_t VEC_SIZE>
tangent<T, VEC_SIZE>::tangent(const tangent<T, VEC_SIZE>& other) = default;

/******************************************************************************
 * @brief Assignment of value to tangent program variable.
 * @param[in] val The passive value.
 ******************************************************************************/
template<typename T, size_t VEC_SIZE>
tangent<T, VEC_SIZE>& tangent<T, VEC_SIZE>::operator=(const T& val) {
   value = val;
   derivative = 0;
   return *this;
}

/******************************************************************************
 * @brief Assignment of value to tangent program variable.
 * @param[in] val The passive value.
 ******************************************************************************/
template<typename T, size_t VEC_SIZE>
template<typename OP, typename... ARGS>
tangent<T, VEC_SIZE>& tangent<T, VEC_SIZE>::operator=(
     const et::expression<OP, ARGS...>& rhs) {

   // Evaluate primal and gradient of the right-hand side of the assignment
   derivative_vector_t<T, VEC_SIZE> d = derivative_vector_t<T, VEC_SIZE>(0.0);
   value = et::ComputeGradient(rhs, T(1.0), d);
   derivative = d;

   return *this;
}

}  // namespace et
}  // namespace ad

// *********************** Include active operations ************************ //

#include "et/operations.hpp"

// ************************************************************************** //

#endif  // AD_ET_TANGENT_HPP_
