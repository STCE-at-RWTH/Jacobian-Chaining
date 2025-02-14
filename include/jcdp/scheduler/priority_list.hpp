#ifndef JCDP_SCHEDULER_PRIORITY_LIST_HPP_
#define JCDP_SCHEDULER_PRIORITY_LIST_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <omp.h>

#include <algorithm>
#include <cstddef>
#include <print>
#include <queue>
#include <numeric>
#include <vector>

#include "jcdp/operation.hpp"
#include "jcdp/scheduler/scheduler.hpp"
#include "jcdp/sequence.hpp"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

namespace jcdp::scheduler {

class PriorityListScheduler : public Scheduler {
 public:
   virtual auto schedule_impl(
        Sequence& sequence, const std::size_t usable_threads,
        const std::size_t) const -> std::size_t override final {

      std::vector<std::size_t> queue_cont(sequence.length());
      std::iota(queue_cont.begin(), queue_cont.end(), 0);

      std::priority_queue queue([&sequence](
         const std::size_t& op_idx1, const std::size_t& op_idx2) -> bool {
            const std::size_t level_1 = sequence.level(op_idx1);
            const std::size_t level_2 = sequence.level(op_idx2);
            if (level_1 == level_2) {
               return sequence.at(op_idx1).fma < sequence.at(op_idx2).fma;
            }
            return sequence.level(op_idx1) < sequence.level(op_idx2);
      }, std::move(queue_cont));

      // Reset potential previous schedule
      for (Operation& op : sequence) {
         op.is_scheduled = false;
      }

      std::vector<std::size_t> thread_loads(usable_threads, 0);
      while (!queue.empty()) {
         const std::size_t op_idx = queue.top();
         const std::size_t earliest_start = sequence.earliest_start(op_idx);

         Operation & op = sequence[op_idx];
         op.thread = 0;
         op.start_time = std::max(thread_loads[0], earliest_start);
         std::size_t current_idle_time = op.start_time - thread_loads[0];

         for (size_t t = 1; t < usable_threads; t++) {
            const std::size_t start_on_t = std::max(thread_loads[t], earliest_start);
            const std::size_t idle_on_t = start_on_t - thread_loads[t];

            if (start_on_t < op.start_time) {
               op.thread = t;
               op.start_time = start_on_t;
               current_idle_time = idle_on_t;
            } else if (start_on_t == op.start_time) {
               if (idle_on_t < current_idle_time) {
                  op.thread = t;
                  op.start_time = start_on_t;
                  current_idle_time = idle_on_t;
               }
            }
         }

         thread_loads[op.thread] = op.start_time + op.fma;
         op.is_scheduled = true;
         queue.pop();
      }

      return sequence.makespan();
   }
};

}  // namespace jcdp::scheduler

// >>>>>>>>>>>>>>>> INCLUDE TEMPLATE AND INLINE DEFINITIONS <<<<<<<<<<<<<<<<< //

// #include "util/impl/jacobian.inl"  // IWYU pragma: export

#endif  // JCDP_SCHEDULER_PRIORITY_LIST_HPP_
