// ************************************************************************** //
//           Advanced Algorithmic Differentiation SS23 - Tutorial 5           //
//                                                                            //
//                        Linear Algebra & Vector mode                        //
//                                                                            //
// Copyright (C) 2023 Software and Tools for Computational Engineering (STCE) //
//               RWTH Aachen University - www.stce.rwth-aachen.de             //
// ************************************************************************** //

#ifndef AD_NOET_DEDICATED_ADJOINT_HPP_
#define AD_NOET_DEDICATED_ADJOINT_HPP_

// Include C++ standard header
#include <cmath>

#include "dedicated_tape.hpp"

// ******************* First order dedicated adjoint type ******************* //

namespace ad {
namespace noet {

/******************************************************************************
 * @brief First order dedicated adjoint type with double precision base type.
 ******************************************************************************/
struct dedicated_adjoint {

   // Default constructors and destructors.
   dedicated_adjoint() = default;
   ~dedicated_adjoint() = default;

   //! Copy constructor.
   dedicated_adjoint(const dedicated_adjoint&);

   //! Constructor with value component.
   explicit dedicated_adjoint(double);

   //! Assignment operator.
   dedicated_adjoint& operator=(const dedicated_adjoint&);

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
   static inline dedicated_tape global_tape;

 private:
   //! Make this adjoint a dedicated adjoint program variable.
   void dedicate_adjoint(const dedicated_adjoint&);
};

/******************************************************************************
 * @brief Constructor with value component
 * @param[in] val The passive value.
 ******************************************************************************/
dedicated_adjoint::dedicated_adjoint(const double val) : value(val) {}

/******************************************************************************
 * @brief Copy constructor.
 * @param[in] arg The other adjoint variable.
 ******************************************************************************/
dedicated_adjoint::dedicated_adjoint(const dedicated_adjoint& arg) = default;

/******************************************************************************
 * @brief Assignment operator.
 *
 * Creates an dedicated adjoint program variable for active left-hand side.
 *
 * @param[in] arg The active variable on the right-hand side.
 ******************************************************************************/
dedicated_adjoint& dedicated_adjoint::operator=(const dedicated_adjoint& arg) {
   if (&arg != this) {
      if (arg.dep_pos >= 0) {
         dedicate_adjoint(arg);
      } else {
         dep_pos = -1;
      }
      value = arg.value;
   }
   return *this;
}

/******************************************************************************
 * @brief Register independent variable.
 ******************************************************************************/
void dedicated_adjoint::register_input() {
   dep_pos = global_tape.dependencies.size();
   global_tape.dependencies.push_back(
        global_tape.persistent_adjoints_counter--);
}

/******************************************************************************
 * @brief Record dependence and derivative of the result of an operation
 *        wrt. an argument.
 *
 * Additionally updates the bandwidth of the tape for non-program variables.
 *
 * @param[in] d Partial derivative.
 ******************************************************************************/
void dedicated_adjoint::record_arg(double d) const {
   if (global_tape.dependencies[dep_pos] >= 0) {
      global_tape.bandwidth = std::max(
           global_tape.bandwidth,
           global_tape.adjoints_counter - global_tape.dependencies[dep_pos]);
   }

   global_tape.dependencies.push_back(global_tape.dependencies[dep_pos]);
   global_tape.derivatives.push_back(d);
}

/******************************************************************************
 * @brief Record the result of an operation and the number of its arguments.
 * @param[in] num_args Number of arguments in the operation.
 ******************************************************************************/
void dedicated_adjoint::record_res(int num_args) {
   global_tape.dependencies.push_back(num_args);
   dep_pos = global_tape.dependencies.size();
   global_tape.dependencies.push_back(global_tape.adjoints_counter++);
}

/******************************************************************************
 * @brief Make this adjoint a dedicated adjoint program variable.
 * @param[in] arg The other adjoint variable (on the right-hand side).
 ******************************************************************************/
void dedicated_adjoint::dedicate_adjoint(const dedicated_adjoint& arg) {
   arg.record_arg(1.0);
   int d = dep_pos;
   record_res(1);

   if (d < 0) {
      global_tape.dependencies[dep_pos] =
           global_tape.persistent_adjoints_counter--;
   } else {
      global_tape.dependencies[dep_pos] = global_tape.dependencies[d];
   }
   global_tape.adjoints_counter--;
}

}  // namespace noet
}  // namespace ad

// *********************** Include active operations ************************ //

#define AD_ADJOINT_T dedicated_adjoint
#include "./active_ops.inl"
#undef AD_ADJOINT_T

// ************************************************************************** //

#endif  // AD_NOET_DEDICATED_ADJOINT_HPP_
