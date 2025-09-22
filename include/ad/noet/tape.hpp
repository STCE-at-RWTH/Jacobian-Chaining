// ************************************************************************** //
//           Advanced Algorithmic Differentiation SS23 - Tutorial 5           //
//                                                                            //
//                        Linear Algebra & Vector mode                        //
//                                                                            //
// Copyright (C) 2023 Software and Tools for Computational Engineering (STCE) //
//               RWTH Aachen University - www.stce.rwth-aachen.de             //
// ************************************************************************** //

#ifndef AD_NOET_TAPE_HPP_
#define AD_NOET_TAPE_HPP_

#include <cstddef>

#include "ad/base_tape.hpp"

// **************************** First order tape **************************** //

namespace ad {
namespace noet {

/******************************************************************************
 * @brief First order adjoint tape based on the base_tape.
 ******************************************************************************/
template<typename T>
struct tape : public base_tape<T> {

   //! Size of vector of adjoints.
   std::size_t derivative_vector_size() override;

   //! Postion in adjoint vector that corresponds to a given tape position.
   std::size_t derivative_idx(std::size_t) const override;
};

/******************************************************************************
 * @brief Size of vector of adjoints.
 * @returns The number of adjoints in the adjoint vector.
 ******************************************************************************/
template<typename T>
std::size_t tape<T>::derivative_vector_size() {
   return this->m_nodes.size();
}

/******************************************************************************
 * @brief Postion in adjoint vector that corresponds to a given tape position.
 * @param[in] tape_index Tape position.
 * @returns The index of the corresponding element in the adjoint vector.
 ******************************************************************************/
template<typename T>
std::size_t tape<T>::derivative_idx(std::size_t tape_index) const {
   return tape_index;
}

}  // namespace noet
}  // namespace ad

// ************************************************************************** //

#endif  // AD_NOET_TAPE_HPP_
