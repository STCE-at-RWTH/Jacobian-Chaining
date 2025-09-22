// ************************************************************************** //
//           Advanced Algorithmic Differentiation SS23 - Tutorial 5           //
//                                                                            //
//                        Linear Algebra & Vector mode                        //
//                                                                            //
// Copyright (C) 2023 Software and Tools for Computational Engineering (STCE) //
//               RWTH Aachen University - www.stce.rwth-aachen.de             //
// ************************************************************************** //

#ifndef AD_NOET_BANDED_TAPE_HPP_
#define AD_NOET_BANDED_TAPE_HPP_

#include "./base_tape.hpp"

// ************************ First order banded tape ************************* //

namespace ad {
namespace noet {

/******************************************************************************
 * @brief First order adjoint banded tape based on the base_tape.
 ******************************************************************************/
struct banded_tape : public base_tape {

   //! Bandwidth of the tape (longest dependency).
   int bandwidth = 0;

   //! Modulo base.
   int mod_base = 0;

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
 * We need at least as many adjoints as the number of independent variables,
 * the number of dependent variables, or the size of the bandwidth.
 *
 * @returns The number of adjoints in the adjoint vector.
 ******************************************************************************/
int banded_tape::num_adjoints() {
   int size = std::max(std::max(bandwidth, num_indeps), num_deps);

#ifdef BITWISE_MODULO
   // Round to the next power of 2
   if ((size & (size - 1)) != 0) {
      int d = std::floor(std::log(size) / std::log(2.0)) + 1;
      size = (1 << d);
   }
#endif

   mod_base = size;
   return size;
}

/******************************************************************************
 * @brief Postion in adjoint vector that corresponds to a given tape position.
 *
 * Utilizes the fact that we can override adjoints in the adjoint vector if
 * the distance between the adjoints is larger than the bandwidth (or rather
 * larger than the size of the adjoint vector). Uses the modulo operation to
 * find the virtual address inside the adjoint vector.
 *
 * @param[in] tape_pos Tape position.
 * @returns The index of the corresponding element in the adjoint vector.
 ******************************************************************************/
int banded_tape::adjoint_idx(int tape_pos) const {
#ifdef BITWISE_MODULO
   return dependencies[tape_pos] & (mod_base - 1);
#else
   return dependencies[tape_pos] % mod_base;
#endif
}

/******************************************************************************
 * @brief Reset the tape.
 ******************************************************************************/
void banded_tape::reset() {
   base_tape::reset();
   bandwidth = 0;
   mod_base = 0;
}

/******************************************************************************
 * @brief Print the size of the tape as well as the adjoint vector.
 ******************************************************************************/
void banded_tape::print_size() const {
   base_tape::print_size();
   std::cerr << "Bandwidth: " << bandwidth << "\n";
}

}  // namespace noet
}  // namespace ad

// ************************************************************************** //

#endif  // AD_NOET_BANDED_TAPE_HPP_
