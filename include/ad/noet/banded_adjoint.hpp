// ************************************************************************** //
//           Advanced Algorithmic Differentiation SS23 - Tutorial 5           //
//                                                                            //
//                        Linear Algebra & Vector mode                        //
//                                                                            //
// Copyright (C) 2023 Software and Tools for Computational Engineering (STCE) //
//               RWTH Aachen University - www.stce.rwth-aachen.de             //
// ************************************************************************** //

#ifndef AD_NOET_BANDED_ADJOINT_HPP_
#define AD_NOET_BANDED_ADJOINT_HPP_

// Include C++ standard header
#include <cmath>

#include "banded_tape.hpp"

// ******************** First order banded adjoint type ********************* //

namespace ad {
namespace noet {

/******************************************************************************
 * @brief First order banded adjoint type with double precision base type.
 ******************************************************************************/
struct banded_adjoint {

   // Default constructors and destructors.
   banded_adjoint() = default;
   ~banded_adjoint() = default;

   //! Constructor with value component.
   explicit banded_adjoint(double);

   //! Register independent variable.
   void register_input();

   //! Record dependence and derivative of the result of an operation
   //! wrt. an argument.
   void record_arg(double) const;

   //! Record the result of an operation and the number of its arguments.
   void record_res(int);

   //! Primal function value.
   double value = 0.0;

   //! Position in dependency vector.
   int dep_pos = -1;

   //! Global tape.
   static inline banded_tape global_tape;
};

/******************************************************************************
 * @brief Constructor with value component
 * @param[in] val The passive value.
 ******************************************************************************/
banded_adjoint::banded_adjoint(const double val) : value(val) {}

/******************************************************************************
 * @brief Register independent variable.
 ******************************************************************************/
void banded_adjoint::register_input() {
   dep_pos = global_tape.dependencies.size();
   global_tape.dependencies.push_back(global_tape.adjoints_counter++);
}

/******************************************************************************
 * @brief Record dependence and derivative of the result of an operation
 *        wrt. an argument.
 *
 * Additionally updates the bandwidth of the tape.
 *
 * @param[in] d Partial derivative.
 ******************************************************************************/
void banded_adjoint::record_arg(double d) const {
   global_tape.bandwidth = std::max(
        global_tape.bandwidth,
        global_tape.adjoints_counter - global_tape.dependencies[dep_pos]);

   global_tape.dependencies.push_back(global_tape.dependencies[dep_pos]);
   global_tape.derivatives.push_back(d);
}

/******************************************************************************
 * @brief Record the result of an operation and the number of its arguments.
 * @param[in] num_args Number of arguments in the operation.
 ******************************************************************************/
void banded_adjoint::record_res(int num_args) {
   global_tape.dependencies.push_back(num_args);
   dep_pos = global_tape.dependencies.size();
   global_tape.dependencies.push_back(global_tape.adjoints_counter++);
}

}  // namespace noet
}  // namespace ad

// *********************** Include active operations ************************ //

#define AD_ADJOINT_T banded_adjoint
#include "./active_ops.inl"
#undef AD_ADJOINT_T

// ************************************************************************** //

#endif  // AD_NOET_BANDED_ADJOINT_HPP_
