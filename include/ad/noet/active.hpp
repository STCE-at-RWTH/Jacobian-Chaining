// ************************************************************************** //
//           Advanced Algorithmic Differentiation SS23 - Tutorial 5           //
//                                                                            //
//                        Linear Algebra & Vector mode                        //
//                                                                            //
// Copyright (C) 2023 Software and Tools for Computational Engineering (STCE) //
//               RWTH Aachen University - www.stce.rwth-aachen.de             //
// ************************************************************************** //

#ifndef AD_NOET_ACTIVE_HPP_
#define AD_NOET_ACTIVE_HPP_

// Include C++ standard header
#include <cstddef>
#include <istream>

#include "tape.hpp"

// ************************ First order adjoint type ************************ //

namespace ad {
namespace noet {

/******************************************************************************
 * @brief First order taped active type with double precision base type.
 ******************************************************************************/
template<typename T>
struct active_t {

   // Default constructors and destructors.
   active_t() = default;
   ~active_t() = default;

   //! Constructor with m_value component.
   explicit active_t(T);

   //! Assignment operator.
   active_t& operator=(const active_t&) = default;

   //! Assignment operator.
   active_t& operator=(T);

   //! Register variable.
   void register_variable();

   //! Register independent variable.
   void register_input();

   //! Register depdent variable.
   void register_output();

   T& value() {
      return m_value;
   }

   const T& value() const {
      return m_value;
   }

   std::size_t tape_index() {
      return m_tape_index;
   }

   //! Record dependence and derivative of the result of an operation
   //! wrt. an argument.
   void record_argument(const active_t<T>&, T) const;

   //! Record the result of an operation and the number of its arguments.
   void record_res(std::size_t);

   //! Primal function m_value.
   T m_value {0.0};

   //! Position in the tape.
   std::size_t m_tape_index {0};

   //! Global tape.
   static inline tape<T> global_tape;
};

/******************************************************************************
 * @brief Constructor with m_value component
 * @param[in] val The passive m_value.
 ******************************************************************************/
template<typename T>
active_t<T>::active_t(const T value) : m_value(value) {}

/******************************************************************************
 * @brief Assignment operator.
 *
 * Creates an dedicated adjoint program variable for active left-hand side.
 *
 * @param[in] arg The active variable on the right-hand side.
 ******************************************************************************/
template<typename T>
active_t<T>& active_t<T>::operator=(const T arg) {
   m_value = arg;
   m_tape_index = 0;
   return *this;
};

/******************************************************************************
 * @brief Register variable.
 ******************************************************************************/
template<typename T>
void active_t<T>::register_variable() {
   m_tape_index = global_tape.create_tape_entry();
}

/******************************************************************************
 * @brief Register independent variable.
 ******************************************************************************/
template<typename T>
void active_t<T>::register_input() {
   register_variable();
   global_tape.mark_independent_variable(m_tape_index);
}

/******************************************************************************
 * @brief Register dependent variable.
 ******************************************************************************/
template<typename T>
void active_t<T>::register_output() {
   global_tape.mark_dependent_variable(m_tape_index);
}

/******************************************************************************
 * @brief Record dependence and derivative of the result of
 an operation
 *        wrt. an argument.
 * @param[in] d Partial derivative.
 ******************************************************************************/
template<typename T>
void active_t<T>::record_argument(
     const active_t<T>& arg, const T partial) const {
   global_tape.record_dependency(m_tape_index, arg.m_tape_index, partial);
}

template<typename T>
static inline std::istream& operator>>(std::istream& is, active_t<T>& var) {
   is >> var.m_value;
   return is;
}

}  // namespace noet
}  // namespace ad

// *********************** Include active operations ************************ //

#define AD_ACTIVE_T active_t
#include "./active_ops.inl"
#undef AD_ACTIVE_T

// ************************************************************************** //

#endif  // AD_NOET_ACTIVE_HPP_
