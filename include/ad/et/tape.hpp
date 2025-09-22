// ************************************************************************** //
//           Advanced Algorithmic Differentiation SS23 - Tutorial 5           //
//                                                                            //
//                        Linear Algebra & Vector mode                        //
//                                                                            //
// Copyright (C) 2023 Software and Tools for Computational Engineering (STCE) //
//               RWTH Aachen University - www.stce.rwth-aachen.de             //
// ************************************************************************** //

#ifndef AD_ET_TAPE_HPP_
#define AD_ET_TAPE_HPP_

#include "base_tape.hpp"

// *********************** First order dedicated tape *********************** //

namespace ad {
namespace et {

/******************************************************************************
 * @brief First order adjoint dedicated tape based on the base_tape.
 ******************************************************************************/
struct tape : public base_tape {

   //! Number of persistent adjoint variables (Counts into negative numbers).
   int persistent_adjoints_counter = -1;

   //! Size of vector of adjoints.
   int num_adjoints() override;

   //! Postion in adjoint vector that corresponds to a given tape position.
   int adjoint_idx(int) const override;

   //! Reset the tape.
   void reset() override;

   //! Print the size of the tape as well as the adjoint vector.
   void print_size() const override;
};

/******************************************************************************
 * @brief Size of vector of adjoints.
 * @returns The number of adjoints in the adjoint vector.
 ******************************************************************************/
int tape::num_adjoints() {
   return -persistent_adjoints_counter - 1;
}

/******************************************************************************
 * @brief Postion in adjoint vector that corresponds to a given tape position.
 *
 * First checks if adjoint is a dedicated adjoint. If yes it returns the
 * dedicated adjoint index.
 *
 * @param[in] tape_pos Tape position.
 * @returns The index of the corresponding element in the adjoint vector.
 ******************************************************************************/
int tape::adjoint_idx(int tape_pos) const {
   int i = dependencies[tape_pos];
   if (i < 0) {
      return -i - 1;
   }
   return i - persistent_adjoints_counter - 1;
}

/******************************************************************************
 * @brief Reset the tape.
 ******************************************************************************/
void tape::reset() {
   base_tape::reset();
   persistent_adjoints_counter = -1;
}

/******************************************************************************
 * @brief Print the size of the tape as well as the adjoint vector.
 ******************************************************************************/
void tape::print_size() const {
   base_tape::print_size();
   std::cerr << "Dedicated adjoints: " << -persistent_adjoints_counter - 1
             << "\n";
}

}  // namespace et
}  // namespace ad

// ************************************************************************** //

#endif  // AD_ET_TAPE_HPP_
