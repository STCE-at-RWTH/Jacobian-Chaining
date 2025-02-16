/******************************************************************************
 * @file jcdp/util/timer.hpp
 *
 * @brief This file is part of the JCDP package. It provides a base class
 *        for a property reader that reads registered properties from an
 *        input file.
 ******************************************************************************/

#ifndef JCDP_UTIL_TIMER_HPP_
#define JCDP_UTIL_TIMER_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <chrono>
#include <list>
#include <print>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "jcdp/util/properties.hpp"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

namespace jcdp::util {

/******************************************************************************
 * @brief Simple timer to limit the time the branch & bound solvers run.
 ******************************************************************************/
class Timer {
 protected:
   using timer_t = std::chrono::steady_clock;
   timer_t::time_point m_start = timer_t::now();
   double m_time_to_solve {-1};
   bool m_timer_expired {false};

 public:
   inline auto set_timer(const double time_to_solve) {
      m_time_to_solve = time_to_solve;
      m_timer_expired = false;
   }

   inline auto start_timer() -> void {
      m_start = timer_t::now();
   }

   inline auto remaining_time() -> double {
      double rem = -1;
      if (m_time_to_solve >= 0) {
         auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
              timer_t::now() - m_start);

         rem = m_time_to_solve;
         rem -= std::min(elapsed.count() / 1'000'000.0, rem);
      }

      m_timer_expired |= !rem;
      return rem;
   }

   inline auto finished_in_time() const -> bool {
      return !m_timer_expired;
   }
};

}  // end namespace jcdp::util

// >>>>>>>>>>>>>>>> INCLUDE TEMPLATE AND INLINE DEFINITIONS <<<<<<<<<<<<<<<<< //

#endif  // JCDP_UTIL_TIMER_HPP_
