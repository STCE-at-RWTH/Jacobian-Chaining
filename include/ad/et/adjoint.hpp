// ************************************************************************** //
//           Advanced Algorithmic Differentiation SS23 - Tutorial 5           //
//                                                                            //
//                        Linear Algebra & Vector mode                        //
//                                                                            //
// Copyright (C) 2023 Software and Tools for Computational Engineering (STCE) //
//               RWTH Aachen University - www.stce.rwth-aachen.de             //
// ************************************************************************** //

#ifndef AD_ET_ADJOINT_HPP_
#define AD_ET_ADJOINT_HPP_

// Include C++ standard header
#include <cmath>

#include "et/expression.hpp"
#include "et/tape.hpp"

// ************************ First order adjoint type ************************ //

namespace ad {
namespace et {

/******************************************************************************
 * @brief First order adjoint type with expression template support.
 ******************************************************************************/
template<typename T>
struct adjoint {

   // Default constructors and destructors.
   adjoint() = default;
   ~adjoint() = default;

   //! Constructor with value component.
   explicit adjoint(T);

   //! Copy constructor.
   adjoint(const adjoint& other);

   //! Assignment of value to adjoint program variable.
   adjoint& operator=(const T& value);

   //! Assignment of expression to adjoint program variable.
   template<typename OP, typename... ARGS>
   adjoint& operator=(const et::expression<OP, ARGS...>& rhs);

   //! Register independent variable.
   void register_input();

   //! Record dependence and derivative of the result of an operation
   //! wrt. an argument.
   void record_arg(double) const;

   //! Record the result of an operation and the number of its arguments.
   void record_res(int);

   //! Primal function value.
   T value = 0.0;

   //! Position in dependency vector.
   int dep_pos = -1;

   //! Global tape.
   static inline tape global_tape;
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
template<typename T>
adjoint<T>::adjoint(const T val) : value(val) {}

/******************************************************************************
 * @brief Copy constructor.
 * @param[in] other The other adjoint variable.
 ******************************************************************************/
template<typename T>
adjoint<T>::adjoint(const adjoint<T>& other) = default;

/******************************************************************************
 * @brief Assignment of value to adjoint program variable.
 * @param[in] val The passive value.
 ******************************************************************************/
template<typename T>
adjoint<T>& adjoint<T>::operator=(const T& val) {
   value = val;
   return *this;
}

/******************************************************************************
 * @brief Assignment of value to adjoint program variable.
 * @param[in] val The passive value.
 ******************************************************************************/
template<typename T>
template<typename OP, typename... ARGS>
adjoint<T>& adjoint<T>::operator=(const et::expression<OP, ARGS...>& rhs) {

   int dep_pos = global_tape.derivatives.size();

   // Evaluate primal and preaccumulate gradient of the right-hand side of the
   // assignment. Adds num_args (see below) dependencies to tape
   T dummy;
   value = et::ComputeGradient(rhs, 1.0, dummy);

   // Update tape dependencies
   int num_args = global_tape.derivatives.size() - dep_pos;
   record_res(num_args);
   return *this;
}

/******************************************************************************
 * @brief Register independent variable.
 ******************************************************************************/
template<typename T>
void adjoint<T>::register_input() {
   dep_pos = global_tape.dependencies.size();
   global_tape.dependencies.push_back(
        global_tape.persistent_adjoints_counter--);
}

/******************************************************************************
 * @brief Record dependence and derivative of the result of an operation
 *        wrt. an argument.
 * @param[in] d Partial derivative.
 ******************************************************************************/
template<typename T>
void adjoint<T>::record_arg(double d) const {
   global_tape.dependencies.push_back(global_tape.dependencies[dep_pos]);
   global_tape.derivatives.push_back(d);
}

/******************************************************************************
 * @brief Record the result of an operation and the number of its arguments.
 * @param[in] num_args Number of arguments in the operation.
 ******************************************************************************/
template<typename T>
void adjoint<T>::record_res(int num_args) {
   if (num_args > 0) {
      global_tape.dependencies.push_back(num_args);
      if (dep_pos == -1) {
         // Result is a new program variable without dedicated adjoint
         register_input();
      } else {
         // Result is a program variable with previously dedicated adjoint
         global_tape.dependencies.push_back(global_tape.dependencies[dep_pos]);
      }
   }
}

}  // namespace et
}  // namespace ad

// *********************** Include active operations ************************ //

#include "et/operations.hpp"

// ************************************************************************** //

#endif  // AD_ET_ADJOINT_HPP_
