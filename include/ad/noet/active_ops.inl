// ************************************************************************** //
//           Advanced Algorithmic Differentiation SS23 - Tutorial 5           //
//                                                                            //
//                        Linear Algebra & Vector mode                        //
//                                                                            //
// Copyright (C) 2023 Software and Tools for Computational Engineering (STCE) //
//               RWTH Aachen University - www.stce.rwth-aachen.de             //
// ************************************************************************** //

// Include C++ standard header
#include <cmath>
#include <iostream>

// The AD_ACTIVE_T<T> has to be defined before including this header file.
// The operations are then defined for the AD_ACTIVE_T.
#ifndef AD_ACTIVE_T
#error "AD_ACTIVE_T not defined!"
#endif

// ********************* First order active operations ********************* //

namespace ad {
namespace noet {

/******************************************************************************
 * @brief Multiplication operator for active types (active * active).
 * @param[in] arg1 The active multiplier.
 * @param[in] arg2 The active multiplicand.
 * @returns An active variable containing the result and derivative of the
 *          active multiplication.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> operator*(const AD_ACTIVE_T<T>& arg1, const AD_ACTIVE_T<T>& arg2) {
   AD_ACTIVE_T<T> res {arg1.m_value * arg2.m_value};
   res.register_variable();

   if (arg1.m_tape_index == arg2.m_tape_index) {
      res.record_argument(arg1, 2 * arg1.m_value);
   } else {
      res.record_argument(arg1, arg2.m_value);
      res.record_argument(arg2, arg1.m_value);
   }
   return res;
}

/******************************************************************************
 * @brief Multiplication operator for active types (passive * active).
 * @param[in] arg1 The passive multiplier.
 * @param[in] arg2 The active multiplicand.
 * @returns An active variable containing the result and derivative of the
 *          active multiplication.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> operator*(const T arg1, const AD_ACTIVE_T<T>& arg2) {
   AD_ACTIVE_T<T> res {arg1 * arg2.m_value};
   res.record_argument(arg2, arg1);
   return res;
}

/******************************************************************************
 * @brief Multiplication operator for active types (active * passive).
 * @param[in] arg1 The active multiplier.
 * @param[in] arg2 The passive multiplicand.
 * @returns An active variable containing the result and derivative of the
 *          active multiplication.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> operator*(const AD_ACTIVE_T<T> arg1, const T& arg2) {
   AD_ACTIVE_T<T> res {arg1.m_value * arg2};
   res.record_argument(arg1, arg2);
   return res;
}

/******************************************************************************
 * @brief Division operator for active types (active / active).
 * @param[in] arg1 The active dividend.
 * @param[in] arg2 The active divisor.
 * @returns An active variable containing the result and derivative of the
 *          active division.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> operator/(const AD_ACTIVE_T<T>& arg1, const AD_ACTIVE_T<T>& arg2) {
   AD_ACTIVE_T<T> res {arg1.m_value / arg2.m_value};
   res.register_variable();
   if (arg1.m_tape_index == arg2.m_tape_index) {
      res.record_argument(arg1, 0.0);
   } else {
      res.record_argument(arg1, 1.0 / arg2.m_value);
      res.record_argument(arg2, -arg1.m_value / (arg2.m_value * arg2.m_value));
   }
   return res;
}

/******************************************************************************
 * @brief Division operator for active types (active / passive).
 * @param[in] arg1 The active dividend.
 * @param[in] arg2 The passive divisor.
 * @returns An active variable containing the result and derivative of the
 *          active division.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> operator/(const AD_ACTIVE_T<T>& arg1, const T arg2) {
   AD_ACTIVE_T<T> res {arg1.m_value / arg2};
   res.record_argument(arg1, 1.0 / arg2);
   return res;
}

/******************************************************************************
 * @brief Division operator for active types (passive / active).
 * @param[in] arg1 The passive dividend.
 * @param[in] arg2 The active divisor.
 * @returns An active variable containing the result and derivative of the
 *          active division.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> operator/(const T arg1, const AD_ACTIVE_T<T>& arg2) {
   AD_ACTIVE_T<T> res {arg1 / arg2.m_value};
   res.register_variable();
   res.record_argument(arg2, -arg1 / (arg2.m_value * arg2.m_value));
   return res;
}

/******************************************************************************
 * @brief Addition operator for active types (active * active).
 * @param[in] arg1 The active augend.
 * @param[in] arg2 The active addend.
 * @returns An active variable containing the result and derivative of the
 *          active addition.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> operator+(const AD_ACTIVE_T<T>& arg1, const AD_ACTIVE_T<T>& arg2) {
   AD_ACTIVE_T<T> res {arg1.m_value + arg2.m_value};
   res.register_variable();

   if (arg1.m_tape_index == arg2.m_tape_index) {
      res.record_argument(arg1, 2);
   } else {
      res.record_argument(arg1, 1);
      res.record_argument(arg2, 1);
   }
   return res;
}

/******************************************************************************
 * @brief Addition operator for active types (active + passive).
 * @param[in] arg1 The active augend.
 * @param[in] arg2 The passive addend.
 * @returns An active variable containing the result and derivative of the
 *          active addition.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> operator+(const AD_ACTIVE_T<T>& arg1, const T arg2) {
   AD_ACTIVE_T<T> res {arg1.m_value + arg2};
   res.record_argument(arg1, 1);
   return res;
}

/******************************************************************************
 * @brief Addition operator for active types (passive + active).
 * @param[in] arg1 The passive augend.
 * @param[in] arg2 The active addend.
 * @returns An active variable containing the result and derivative of the
 *          active addition.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> operator+(const T arg1, const AD_ACTIVE_T<T>& arg2) {
   AD_ACTIVE_T<T> res {arg1 + arg2.m_value};
   res.record_argument(arg2, 1.0);
   return res;
}

/******************************************************************************
 * @brief Unary plus operator for active types.
 * @param[in] arg The active variable.
 * @returns An active variable containing the same value and derivative.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> operator+(const AD_ACTIVE_T<T>& arg) {
   AD_ACTIVE_T<T> res {arg.m_value};
   res.register_variable();
   res.record_argument(arg, 1.0);
   return res;
}

/******************************************************************************
 * @brief Subtraction operator for active types (active - active).
 * @param[in] arg1 The active minuend.
 * @param[in] arg2 The active subtrahend.
 * @returns An active variable containing the result and derivative of the
 *          active subtraction.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> operator-(const AD_ACTIVE_T<T>& arg1, const AD_ACTIVE_T<T>& arg2) {
   AD_ACTIVE_T<T> res {arg1.m_value - arg2.m_value};
   res.register_variable();
   if (arg1.m_tape_index == arg2.m_tape_index) {
      res.record_argument(arg1, 0);
   } else {
      res.record_argument(arg1, 1);
      res.record_argument(arg2, -1);
   }
   return res;
}

/******************************************************************************
 * @brief Subtraction operator for active types (active - passive).
 * @param[in] arg1 The active minuend.
 * @param[in] arg2 The active subtrahend.
 * @returns An active variable containing the result and derivative of the
 *          active subtraction.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> operator-(const AD_ACTIVE_T<T>& arg1, const T arg2) {
   AD_ACTIVE_T<T> res {arg1.m_value - arg2};
   res.record_argument(arg1, 1);
   return res;
}

/******************************************************************************
 * @brief Subtraction operator for active types (passive - active).
 * @param[in] arg1 The passive minuend.
 * @param[in] arg2 The active subtrahend.
 * @returns An active variable containing the result and derivative of the
 *          active subtraction.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> operator-(const T arg1, const AD_ACTIVE_T<T>& arg2) {
   AD_ACTIVE_T<T> res {arg1 - arg2.m_value};
   res.register_variable();
   res.record_argument(arg2, -1);
   return res;
}

/******************************************************************************
 * @brief Negation operator for active types.
 * @param[in] arg The active variable to negate.
 * @returns An active variable containing the negated value and derivative.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> operator-(const AD_ACTIVE_T<T>& arg) {
   AD_ACTIVE_T<T> res {-arg.m_value};
   res.register_variable();
   res.record_argument(arg, -1.0);
   return res;
}

/******************************************************************************
 * @brief Sine function for active types.
 * @param[in] arg The active input.
 * @returns An active variable containing the result and derivative of the
 *          sine function.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> sin(const AD_ACTIVE_T<T>& arg) {
   AD_ACTIVE_T<T> res {std::sin(arg.m_value)};
   res.register_variable();
   res.record_argument(arg, std::cos(arg.m_value));
   return res;
}

/******************************************************************************
 * @brief Cosine function for active types.
 * @param[in] arg The active input.
 * @returns An active variable containing the result and derivative of the
 *          cosine function.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> cos(const AD_ACTIVE_T<T>& arg) {
   AD_ACTIVE_T<T> res {std::cos(arg.m_value)};
   res.register_variable();
   res.record_argument(arg, -std::sin(arg.m_value));
   return res;
}

/******************************************************************************
 * @brief Tangent function for active types.
 * @param[in] arg The active input.
 * @returns An active variable containing the result and derivative of the
 *          tangent function.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> tan(const AD_ACTIVE_T<T>& arg) {
   AD_ACTIVE_T<T> res {std::tan(arg.m_value)};
   res.register_variable();
   res.record_argument(arg, 1.0 + (res.m_value * res.m_value));
   return res;
}

/******************************************************************************
 * @brief Exponential function for active types.
 * @param[in] arg The active input.
 * @returns An active variable containing the result and derivative of the
 *          exponential function.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> exp(const AD_ACTIVE_T<T>& arg) {
   AD_ACTIVE_T<T> res {std::exp(arg.m_value)};
   res.register_variable();
   res.record_argument(arg, res.m_value);
   return res;
}

/******************************************************************************
 * @brief Logarithm function for active types.
 * @param[in] arg The active input.
 * @returns An active variable containing the result and derivative of the
 *          logarithm function.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> log(const AD_ACTIVE_T<T>& arg) {
   AD_ACTIVE_T<T> res {std::log(arg.m_value)};
   res.register_variable();
   res.record_argument(arg, 1.0 / arg.m_value);
   return res;
}

/******************************************************************************
 * @brief Square root function for active types.
 * @param[in] arg The active input.
 * @returns An active variable containing the result and derivative of the
 *          square root function.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> sqrt(const AD_ACTIVE_T<T>& arg) {
   AD_ACTIVE_T<T> res {std::sqrt(arg.m_value)};
   res.register_variable();
   res.record_argument(arg, 0.5 / res.m_value);
   return res;
}

/******************************************************************************
 * @brief Power function for active types (active / active).
 * @param[in] arg1 The active base.
 * @param[in] arg2 The active exponent.
 * @returns An active variable containing the result and derivative of the
 *          power function.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> pow(const AD_ACTIVE_T<T>& arg1, const AD_ACTIVE_T<T>& arg2) {
   AD_ACTIVE_T<T> res {std::pow(arg1.m_value, arg2.m_value)};
   res.register_variable();

   if (arg2.m_value == 0.0) {
      res.record_argument(arg1, 0.0);
   } else {
      res.record_argument(arg1, arg2.m_value * std::pow(arg1.m_value, arg2.m_value - 1));
   }
   res.record_argument(arg2, std::log(arg1.m_value) * res.m_value);
   return res;
}

/******************************************************************************
 * @brief Power function for active types (active / passive).
 * @param[in] arg1 The active base.
 * @param[in] arg2 The passive exponent.
 * @returns An active variable containing the result and derivative of the
 *          power function.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> pow(const AD_ACTIVE_T<T>& arg1, const T arg2) {
   AD_ACTIVE_T<T> res {std::pow(arg1.m_value, arg2)};
   res.register_variable();

   if (arg2 == 0.0) {
      res.record_argument(arg1, 0.0);
   } else {
      res.record_argument(arg1, arg2 * std::pow(arg1.m_value, arg2 - 1));
   }
   return res;
}

/******************************************************************************
 * @brief Power function for active types (passive / active).
 * @param[in] arg1 The passive base.
 * @param[in] arg2 The active exponent.
 * @returns An active variable containing the result and derivative of the
 *          power function.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> pow(const T arg1, const AD_ACTIVE_T<T>& arg2) {
   AD_ACTIVE_T<T> res {std::pow(arg1, arg2.m_value)};
   res.register_variable();
   res.record_argument(arg2, std::log(arg1) * res.m_value);
   return res;
}

/******************************************************************************
 * @brief Abs function for active types.
 * @param[in] arg The active input.
 * @returns An active variable containing the result and derivative of the
 *          absolute value function.
 ******************************************************************************/
template<typename T>
AD_ACTIVE_T<T> abs(const AD_ACTIVE_T<T>& arg) {
   AD_ACTIVE_T<T> res {std::abs(arg.m_value)};
   res.register_variable();
   if (arg.m_value >= 0.0) {
      res.record_argument(arg, 1.0);
   } else {
      res.record_argument(arg, -1.0);
   }
   return res;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>> Relational Operators <<<<<<<<<<<<<<<<<<<<<<<<<< //

/******************************************************************************
 * @brief Equality operator for active types.
 * @param[in] arg1 The first active variable.
 * @param[in] arg2 The second active variable.
 * @returns True if both active variables are equal, false otherwise.
 ******************************************************************************/
template<typename T>
bool operator==(const AD_ACTIVE_T<T>& arg1, const AD_ACTIVE_T<T>& arg2) {
   return (arg1.m_value == arg2.m_value);
}

/******************************************************************************
 * @brief Equality operator for active types (active == passive).
 * @param[in] arg1 The active variable.
 * @param[in] arg2 The passive variable.
 * @returns True if both active and passive variables are equal, false otherwise.
 ******************************************************************************/
template<typename T>
bool operator==(const AD_ACTIVE_T<T>& arg1, const T arg2) {
   return (arg1.m_value == arg2);
}

/******************************************************************************
 * @brief Equality operator for active types (passive == active).
 * @param[in] arg1 The passive variable.
 * @param[in] arg2 The active variable.
 * @returns True if both passive and active variables are equal, false otherwise.
 ******************************************************************************/
template<typename T>
bool operator==(const T arg1, const AD_ACTIVE_T<T>& arg2) {
   return (arg1 == arg2.m_value);
}

/******************************************************************************
 * @brief Inequality operator for active types.
 * @param[in] arg1 The first active variable.
 * @param[in] arg2 The second active variable.
 * @returns True if both active variables are not equal, false otherwise.
 ******************************************************************************/
template<typename T>
bool operator!=(const AD_ACTIVE_T<T>& arg1, const AD_ACTIVE_T<T>& arg2) {
   return (arg1.m_value != arg2.m_value);
}

/******************************************************************************
 * @brief Inequality operator for active types (active != passive).
 * @param[in] arg1 The active variable.
 * @param[in] arg2 The passive variable.
 * @returns True if both active and passive variables are not equal, false otherwise.
 ******************************************************************************/
template<typename T>
bool operator!=(const AD_ACTIVE_T<T>& arg1, const T arg2) {
   return (arg1.m_value != arg2);
}

/******************************************************************************
 * @brief Inequality operator for active types (passive != active).
 * @param[in] arg1 The passive variable.
 * @param[in] arg2 The active variable.
 * @returns True if both passive and active variables are not equal, false otherwise.
 ******************************************************************************/
template<typename T>
bool operator!=(const T arg1, const AD_ACTIVE_T<T>& arg2) {
   return (arg1 != arg2.m_value);
}

/******************************************************************************
 * @brief Less than operator for active types.
 * @param[in] arg1 The first active variable.
 * @param[in] arg2 The second active variable.
 * @returns True if the first active variable is less than the second, false otherwise.
 ******************************************************************************/
template<typename T>
bool operator<(const AD_ACTIVE_T<T>& arg1, const AD_ACTIVE_T<T>& arg2) {
   return (arg1.m_value < arg2.m_value);
}

/******************************************************************************
 * @brief Less than operator for active types (active < passive).
 * @param[in] arg1 The active variable.
 * @param[in] arg2 The passive variable.
 * @returns True if the active variable is less than the passive variable, false otherwise.
 ******************************************************************************/
template<typename T>
bool operator<(const AD_ACTIVE_T<T>& arg1, const T arg2) {
   return (arg1.m_value < arg2);
}

/******************************************************************************
 * @brief Less than operator for active types (passive < active).
 * @param[in] arg1 The passive variable.
 * @param[in] arg2 The active variable.
 * @returns True if the passive variable is less than the active variable, false otherwise.
 ******************************************************************************/
template<typename T>
bool operator<(const T arg1, const AD_ACTIVE_T<T>& arg2) {
   return (arg1 < arg2.m_value);
}

/******************************************************************************
 * @brief Less than or equal operator for active types.
 * @param[in] arg1 The first active variable.
 * @param[in] arg2 The second active variable.
 * @returns True if the first active variable is less than or equal to the second, false otherwise.
 ******************************************************************************/
template<typename T>
bool operator<=(const AD_ACTIVE_T<T>& arg1, const AD_ACTIVE_T<T>& arg2) {
   return (arg1.m_value <= arg2.m_value);
}

/******************************************************************************
 * @brief Less than or equal operator for active types (active <= passive).
 * @param[in] arg1 The active variable.
 * @param[in] arg2 The passive variable.
 * @returns True if the active variable is less than or equal to the passive variable, false otherwise.
 ******************************************************************************/
template<typename T>
bool operator<=(const AD_ACTIVE_T<T>& arg1, const T arg2) {
   return (arg1.m_value <= arg2);
}

/******************************************************************************
 * @brief Less than or equal operator for active types (passive <= active).
 * @param[in] arg1 The passive variable.
 * @param[in] arg2 The active variable.
 * @returns True if the passive variable is less than or equal to the active variable, false otherwise.
 ******************************************************************************/
template<typename T>
bool operator<=(const T arg1, const AD_ACTIVE_T<T>& arg2) {
   return (arg1 <= arg2.m_value);
}

/******************************************************************************
 * @brief Greater than operator for active types.
 * @param[in] arg1 The first active variable.
 * @param[in] arg2 The second active variable.
 * @returns True if the first active variable is greater than the second, false otherwise.
 ******************************************************************************/
template<typename T>
bool operator>(const AD_ACTIVE_T<T>& arg1, const AD_ACTIVE_T<T>& arg2) {
   return (arg1.m_value > arg2.m_value);
}

/******************************************************************************
 * @brief Greater than operator for active types (active > passive).
 * @param[in] arg1 The active variable.
 * @param[in] arg2 The passive variable.
 * @returns True if the active variable is greater than the passive variable, false otherwise.
 ******************************************************************************/
template<typename T>
bool operator>(const AD_ACTIVE_T<T>& arg1, const T arg2) {
   return (arg1.m_value > arg2);
}

/******************************************************************************
 * @brief Greater than operator for active types (passive > active).
 * @param[in] arg1 The passive variable.
 * @param[in] arg2 The active variable.
 * @returns True if the passive variable is greater than the active variable, false otherwise.
 ******************************************************************************/
template<typename T>
bool operator>(const T arg1, const AD_ACTIVE_T<T>& arg2) {
   return (arg1 > arg2.m_value);
}

/******************************************************************************
 * @brief Greater than or equal operator for active types.
 * @param[in] arg1 The first active variable.
 * @param[in] arg2 The second active variable.
 * @returns True if the first active variable is greater than or equal to the second, false otherwise.
 ******************************************************************************/
template<typename T>
bool operator>=(const AD_ACTIVE_T<T>& arg1, const AD_ACTIVE_T<T>& arg2) {
   return (arg1.m_value >= arg2.m_value);
}

/******************************************************************************
 * @brief Greater than or equal operator for active types (active >= passive).
 * @param[in] arg1 The active variable.
 * @param[in] arg2 The passive variable.
 * @returns True if the active variable is greater than or equal to the passive variable, false otherwise.
 ******************************************************************************/
template<typename T>
bool operator>=(const AD_ACTIVE_T<T>& arg1, const T arg2) {
   return (arg1.m_value >= arg2);
}

/******************************************************************************
 * @brief Greater than or equal operator for active types (passive >= active).
 * @param[in] arg1 The passive variable.
 * @param[in] arg2 The active variable.
 * @returns True if the passive variable is greater than or equal to the active variable, false otherwise.
 ******************************************************************************/
template<typename T>
bool operator>=(const T arg1, const AD_ACTIVE_T<T>& arg2) {
   return (arg1 >= arg2.m_value);
}

/******************************************************************************
 * @brief Output stream to print a active variable.
 * @param[inout] out Reference to the output stream.
 * @param[in] arg The active variable.
 * @returns The reference to the output stream.
 ******************************************************************************/
template<typename T>
std::ostream& operator<<(std::ostream& out, const AD_ACTIVE_T<T>& arg) {
   out << "{m_value: " << arg.m_value << ", position: " << arg.m_tape_index << "}";
   return out;
}

}  // namespace noet
}  // namespace ad

// ************************************************************************** //
