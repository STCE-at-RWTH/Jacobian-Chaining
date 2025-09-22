// ************************************************************************** //
//           Advanced Algorithmic Differentiation SS23 - Tutorial 5           //
//                                                                            //
//                        Linear Algebra & Vector mode                        //
//                                                                            //
// Copyright (C) 2023 Software and Tools for Computational Engineering (STCE) //
//               RWTH Aachen University - www.stce.rwth-aachen.de             //
// ************************************************************************** //

#ifndef AD_NOET_DEDICATED_TAPE_HPP_
#define AD_NOET_DEDICATED_TAPE_HPP_

#include "./base_tape.hpp"

// *********************** First order dedicated tape *********************** //

namespace ad {
namespace noet {

/******************************************************************************
 * @brief First order adjoint dedicated tape based on the base_tape.
 ******************************************************************************/
struct dedicated_tape : public base_tape {

   //! Bandwidth of the tape (longest dependency).
   int bandwidth = 0;

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
 *
 * We need at least as many adjoints as the number of dedicated adjoint
 * variables plus the size of the bandwidth.
 *
 * @returns The number of adjoints in the adjoint vector.
 ******************************************************************************/
int dedicated_tape::num_adjoints() {

#ifdef BITWISE_MODULO
   // Round to the next power of 2
   if ((bandwidth & (bandwidth - 1)) != 0) {
      int d = std::floor(std::log(bandwidth) / std::log(2.0)) + 1;
      bandwidth = (1 << d);
   }
#endif

   return bandwidth - persistent_adjoints_counter - 1;
}

/******************************************************************************
 * @brief Postion in adjoint vector that corresponds to a given tape position.
 *
 * First checks if adjoint is a dedicated adjoint. If yes it returns the
 * dedicated adjoint index. If not it utilizes the fact that we can override
 * adjoints in the adjoint vector if the distance between the adjoints is
 * larger than the bandwidth. Uses the modulo operation to find the virtual
 * address inside the adjoint vector and shifts the position outside the range
 * if dedicated adjoint variables.
 *
 * @param[in] tape_pos Tape position.
 * @returns The index of the corresponding element in the adjoint vector.
 ******************************************************************************/
int dedicated_tape::adjoint_idx(int tape_pos) const {
   int i = dependencies[tape_pos];
   if (i < 0) {
      return -i - 1;
   }

#ifdef BITWISE_MODULO
   return (i & (bandwidth - 1)) - persistent_adjoints_counter - 1;
#else
   return i % bandwidth - persistent_adjoints_counter - 1;
#endif
}

/******************************************************************************
 * @brief Reset the tape.
 ******************************************************************************/
void dedicated_tape::reset() {
   base_tape::reset();
   bandwidth = 0;
   persistent_adjoints_counter = -1;
}

/******************************************************************************
 * @brief Print the size of the tape as well as the adjoint vector.
 ******************************************************************************/
void dedicated_tape::print_size() const {
   base_tape::print_size();
   std::cerr << "Bandwidth: " << bandwidth << "\n";
   std::cerr << "Dedicated adjoints: " << -persistent_adjoints_counter - 1
             << "\n";
}

}  // namespace noet
}  // namespace ad

// ************************************************************************** //

#endif  // AD_NOET_DEDICATED_TAPE_HPP_
