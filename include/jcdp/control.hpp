#ifndef JCDP_CONTROL_HPP_
#define JCDP_CONTROL_HPP_

#include <cstddef>
#include <cstdint>
#include <print>
#include <vector>

#include "jcdp/sequence.hpp"
#include "jcdp/util/timer.hpp"

namespace jcdp {

/**
 * @brief Enum to represent the state control for JCDP execution.
 */
enum class StateControl : int32_t {
   RUN = 0,
   PAUSE = 1,
   CANCEL = 2,
   RESTART = 3,
   DONE = 4
};

/**
 * @brief Struct to hold statistics and control flags for the B&B execution.
 */
struct SolverState {
   std::uint32_t visited_leafs {0};
   std::uint32_t updated_makespans {0};
   std::uint32_t pruned_branches {0};
   volatile StateControl state {StateControl::RUN};
   double runtime_ms {0};
   double estimated_search_space {0};
   double explored_search_space {0};
   const char* result_ptr;

   Sequence optimal_sequence {Sequence::make_max()};
   std::string optimal_sequence_json {};
   std::vector<bool> finished_level_1_tasks {};
   std::vector<std::size_t> pruned_branches_per_length {};

   /**
    * @brief Checks the current state and acts accordingly.
    *
    * @return true if execution should continue, false if it should be
    * cancelled.
    */
   inline auto barrier(util::Timer* timer) const -> bool {
      if (state == StateControl::PAUSE) {
         #pragma omp critical (timer_pause)
         timer->pause_timer();

         while (state == StateControl::PAUSE) {
            #pragma omp taskyield
         }

         #pragma omp critical (timer_pause)
         timer->resume_timer();
      }

      if (state == StateControl::CANCEL) {
         return false;
      }
      return true;
   }

   /**
    * @brief Prints the collected statistics to stdout.
    */
   inline auto print_stats() -> void {
      std::println("Leafs visited (= sequences scheduled): {}", visited_leafs);
      std::println("Updated makespan: {}", updated_makespans);
      std::println(
           "Pruned branches: {}", std::reduce(
                                       pruned_branches_per_length.cbegin(),
                                       pruned_branches_per_length.cend()));
      std::println("Pruned branches per sequence length:");
      std::print("[ ");
      for (const std::size_t pruned : pruned_branches_per_length) {
         std::print("{} ", pruned);
      }
      std::println("]");
   }
};

}  // namespace jcdp

#endif  // JCDP_CONTROL_HPP_
