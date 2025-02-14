#ifndef JCDP_SCHEDULER_SCHEDULER_HPP_
#define JCDP_SCHEDULER_SCHEDULER_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <cstddef>
#include <limits>
#include <print>

#include "jcdp/operation.hpp"
#include "jcdp/sequence.hpp"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

namespace jcdp::scheduler {

class Scheduler {
 public:
   Scheduler() = default;
   virtual ~Scheduler() = default;

   inline auto schedule(
        Sequence& sequence, const std::size_t threads,
        const std::size_t upper_bound = std::numeric_limits<std::size_t>::max()) const -> std::size_t {

      // We can never use more threads than we have accumulations
      std::size_t usable_threads = 0;
      for (Operation& op : sequence) {
         usable_threads += (op.action == Action::ACCUMULATION);
      }
      if (threads > 0 && threads < usable_threads) {
         usable_threads = threads;
      }

      return schedule_impl(sequence, usable_threads, upper_bound);
   }

   virtual auto schedule_impl(
        Sequence&, const std::size_t, const std::size_t) const
        -> std::size_t = 0;
};

}  // namespace jcdp::scheduler

// >>>>>>>>>>>>>>>> INCLUDE TEMPLATE AND INLINE DEFINITIONS <<<<<<<<<<<<<<<<< //

// #include "util/impl/jacobian.inl"  // IWYU pragma: export

#endif  // JCDP_SCHEDULER_SCHEDULER_HPP_
