// ************************************************************************** //
//           Advanced Algorithmic Differentiation SS23 - Tutorial 5           //
//                                                                            //
//                        Linear Algebra & Vector mode                        //
//                                                                            //
// Copyright (C) 2023 Software and Tools for Computational Engineering (STCE) //
//               RWTH Aachen University - www.stce.rwth-aachen.de             //
// ************************************************************************** //

#ifndef AD_VECTOR_HPP_
#define AD_VECTOR_HPP_

// Include eigen
#include <Eigen/Dense>

namespace ad {
namespace helper {

/******************************************************************************
 * @brief Struct that defines derivative and adjoint vector types.
 * @tparam T Passive type.
 * @tparam VEC_SIZE Vector size.
 ******************************************************************************/
template<typename T, size_t VEC_SIZE>
struct types {

   //! Type for derivative vectors in tangent types (row vectors).
   using vector_t = Eigen::Array<T, 1, VEC_SIZE>;

   //! Type for adjoint vectors/matrices in taped types (each row is
   //! a derivative vector).
   using derivative_vector_t = Eigen::Array<T, Eigen::Dynamic, VEC_SIZE>;
};

/******************************************************************************
 * @brief Specialization for vector size of 1. (DOESN'T work at the moment)
 * @tparam T Passive type.
 ******************************************************************************/
// template<typename T>
// struct types<T, 1> {
//   using derivative_vector = T;
//   using adjoint_vector = Eigen::Array<T, Eigen::Dynamic, 1>;
// };

}  // namespace helper

//! Generic derivative vector typedef for convinience.
template<typename T, size_t VEC_SIZE>
using vector_t = typename helper::types<T, VEC_SIZE>::vector_t;

//! Generic adjoint vector typedef for convinience.
template<typename T, size_t VEC_SIZE>
using derivative_vector_t =
     typename helper::types<T, VEC_SIZE>::derivative_vector_t;

}  // namespace ad

#endif  // AD_VECTOR_HPP_
