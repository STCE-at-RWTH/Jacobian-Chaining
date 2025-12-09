/******************************************************************************
 * @file jcdp/util/timer.hpp
 *
 * @brief This file is part of the JCDP package. It provides a timer class
 *        which is used in the solvers to limit the time they can use.
 ******************************************************************************/

#ifndef JCDP_UTIL_TIMER_HPP_
#define JCDP_UTIL_TIMER_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <chrono>
#include <print>

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

namespace jcdp::util {

/******************************************************************************
 * @brief Simple timer to limit the time the branch & bound solvers run.
 ******************************************************************************/
class Timer {
 protected:
   using timer_t = std::chrono::steady_clock;
   timer_t::time_point m_start = timer_t::now();
   timer_t::time_point m_pause_start = timer_t::now();
   std::chrono::microseconds m_paused_duration =
        std::chrono::microseconds::zero();
   double m_time_to_solve {-1};
   bool m_timer_expired {false};

 public:
   inline auto set_timer(const double time_to_solve) {
      m_time_to_solve = time_to_solve;
      m_timer_expired = false;
      m_paused_duration = std::chrono::microseconds::zero();
   }

   inline auto start_timer() -> void {
      m_start = timer_t::now();
   }

   inline auto pause_timer() -> void {
      m_pause_start = timer_t::now();
   }

   inline auto resume_timer() -> void {
      m_paused_duration +=
           std::chrono::duration_cast<std::chrono::microseconds>(
                timer_t::now() - m_pause_start);
   }

   inline auto elapsed_time() -> double {
      auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
           timer_t::now() - m_start) - m_paused_duration;
      return elapsed.count();
   }

   inline auto remaining_time() -> double {
      double rem = -1;
      if (m_time_to_solve >= 0) {
         rem = m_time_to_solve;
         rem -= std::min(elapsed_time() / 1'000'000.0, rem);
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
