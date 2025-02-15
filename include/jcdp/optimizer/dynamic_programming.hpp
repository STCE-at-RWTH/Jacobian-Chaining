/******************************************************************************
 * @file jcdp/optimizer/dynamic_programming.hpp
 *
 * @brief This file is part of the JCDP package. It provides an optimizer that
 *        uses a dynamic programming algorithm to find the best possible
 *        brackating (elimination sequence) for a given Jacobian chain.
 *        Optimality is only given with one or unlimited threads.
 ******************************************************************************/

#ifndef JCDP_OPTIMIZER_DYNAMIC_PROGRAMMING_HPP_
#define JCDP_OPTIMIZER_DYNAMIC_PROGRAMMING_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <optional>
#include <print>
#include <utility>
#include <vector>

#include "jcdp/jacobian.hpp"
#include "jcdp/jacobian_chain.hpp"
#include "jcdp/operation.hpp"
#include "jcdp/optimizer/optimizer.hpp"
#include "jcdp/sequence.hpp"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

namespace jcdp::optimizer {

struct DPNode {
   Operation op {};
   std::size_t cost {std::numeric_limits<std::size_t>::max()};
   std::size_t thread_split {0};
   bool visited {false};
};

class DynamicProgrammingOptimizer : public Optimizer {
 public:
   DynamicProgrammingOptimizer() = default;

   virtual auto init(const JacobianChain& chain) -> void override final {
      Optimizer::init(chain);

      std::size_t dp_nodes = m_length * (m_length + 1) / 2;
      if (m_usable_threads > 0) {
         dp_nodes *= m_usable_threads;

         // Correct for preaccumulation nodes which only ever use one thread
         dp_nodes -= (m_usable_threads - 1) * m_length;
      }

      std::println("Dynamic Programming nodes: {}", dp_nodes);
      m_dptable.clear();
      m_dptable.resize(dp_nodes);
   }

   virtual auto solve() -> Sequence override final {
      const std::ptrdiff_t j_max = static_cast<std::ptrdiff_t>(m_length);

      // Accumulation costs
      #pragma omp parallel for
      for (std::ptrdiff_t j = 0; j < j_max; ++j) {
         try_accumulation<Mode::TANGENT>(j);
         try_accumulation<Mode::ADJOINT>(j);
      }

      // Iterate over amount of available threads. m_usable_threads can be 0
      // which means unlimited threads, therefore using do-while loop here.
      std::size_t threads = 1;
      do {
         for (std::size_t len = 2; len <= m_length; ++len) {
            // Chains with same lengths and threads are independent!
            #pragma omp parallel for
            for (std::ptrdiff_t j = len - 1; j < j_max; ++j) {
               const std::ptrdiff_t i = j - (len - 1);

               for (std::ptrdiff_t k = i; k < j; k++) {
                  try_multiplication(j, i, k, threads);

                  if (m_matrix_free) {
                     try_elimination<Mode::TANGENT>(j, i, k, threads);

                     // Search for adjoint elimination from the back to the
                     // to get the longest adjoint elimination chain possible.
                     // Otherwise we get a lot of single adjoint eliminations
                     // one after another. Doesn't affect fma, just reduces work
                     // and makes output smaller.
                     const std::size_t k2 = j - (k - i + 1);
                     try_elimination<Mode::ADJOINT>(j, i, k2, threads);
                  }
               }
            }
         }
      } while (++threads <= m_usable_threads);

      return get_sequence();
   }

   auto get_sequence(const std::optional<std::size_t> threads = {})
        -> Sequence {
      Sequence seq {};
      build_sequence(
           m_length - 1, 0, {0, threads.value_or(m_usable_threads) - 1}, seq);
      return seq;
   }

   auto build_sequence(
        const std::size_t j, const std::size_t i,
        const std::pair<std::size_t, std::size_t> thread_pool, Sequence& seq,
        std::size_t start_time = 0) -> std::size_t {

      const std::size_t t = thread_pool.second - thread_pool.first + 1;
      DPNode& fma_ji = node(j, i, t);
      assert(fma_ji.visited);

      switch (fma_ji.op.action) {
         case Action::ACCUMULATION: {
            fma_ji.op.thread = thread_pool.first;
            if (m_usable_threads > 0) {
               fma_ji.op.start_time = std::max(
                  seq.makespan(fma_ji.op.thread), start_time);
            } else {
               fma_ji.op.start_time = 0;
            }
         } break;

         case Action::MULTIPLICATION: {
            std::pair<std::size_t, std::size_t> thread_pool_jk = thread_pool;
            std::pair<std::size_t, std::size_t> thread_pool_ki = thread_pool;
            if (fma_ji.thread_split > 0) {
               thread_pool_ki.first = thread_pool.first + fma_ji.thread_split;
               thread_pool_jk.second = thread_pool_ki.first - 1;
            }
            const std::size_t jk_end_time = build_sequence(
                 j, fma_ji.op.k + 1, thread_pool_jk, seq, start_time);

            // fma_ji.thread_split == 0 means we perform fma_jk and fma_ki in
            // serial. Therefore update the start time for fma_ki. This can lead
            // to a suboptimal schdule and we should reschedule the sequence
            // with branch & bound as a post-processing step!
            if (fma_ji.thread_split == 0) {
               start_time = jk_end_time;
            }

            const std::size_t ki_end_time = build_sequence(
                 fma_ji.op.k, i, thread_pool_ki, seq, start_time);

            if (jk_end_time >= ki_end_time) {
               fma_ji.op.thread = thread_pool_jk.first;
               fma_ji.op.start_time = jk_end_time;
            } else {
               fma_ji.op.thread = thread_pool_ki.first;
               fma_ji.op.start_time = ki_end_time;
            }
         } break;

         case Action::ELIMINATION: {
            std::size_t end_time;
            if (fma_ji.op.mode == Mode::TANGENT) {
               end_time = build_sequence(
                    fma_ji.op.k, i, thread_pool, seq, start_time);
            } else {
               end_time = build_sequence(
                    j, fma_ji.op.k + 1, thread_pool, seq, start_time);
            }

            fma_ji.op.thread = thread_pool.first;
            fma_ji.op.start_time = end_time;
         } break;

         default: {
            assert(false);
         }
      }

      fma_ji.op.is_scheduled = true;
      seq += fma_ji.op;
      return seq.back().start_time + seq.back().fma;
   }

 private:
   std::vector<DPNode> m_dptable;

   auto node(const std::size_t j, const std::size_t i, const std::size_t t)
        -> DPNode& {
      assert(j < m_length);
      assert(i < m_length && i <= j);

      std::size_t idx = j * (j + 1) / 2 + i;
      if (m_usable_threads > 0 && j != i) {
         assert(t <= m_usable_threads);
         idx += (t - 1) * (m_length + 1) * (m_length) / 2;

         // Correct for preaccumulation nodes which only ever use one thread
         if (t >= 2) {
            idx -= (t - 2) * m_length + j;
         }
      }
      return m_dptable[idx];
   }

   template<Mode mode>
   auto try_accumulation(const std::size_t j) -> void {
      std::size_t fma;
      if constexpr (mode == Mode::ADJOINT) {
         if (m_available_memory > 0) {
            const std::size_t mem = m_chain.get_jacobian(j, j).edges_in_dag;
            if (mem > m_available_memory) {
               return;
            }
         }
      }

      fma = m_chain.get_jacobian(j, j).fma<mode>();
      DPNode& fma_ji = node(j, j, 1);
      if (fma < fma_ji.cost) {
         fma_ji.op.action = Action::ACCUMULATION;
         fma_ji.op.mode = mode;
         fma_ji.op.fma = fma;
         fma_ji.op.i = j;
         fma_ji.op.j = j;
         fma_ji.op.k = j;
         fma_ji.cost = fma;
         fma_ji.thread_split = 0;
         fma_ji.visited = true;
      }
   }

   auto try_multiplication(
        const std::size_t j, const std::size_t i, const std::size_t k,
        const std::size_t t) -> void {
      std::size_t cost = std::numeric_limits<std::size_t>::max();
      std::size_t thread_split = 0;

      // Perform fma_jk and fma_ki in serial
      {
         const DPNode& fma_jk = node(j, k + 1, t);
         const DPNode& fma_ki = node(k, i, t);
         assert(fma_jk.visited);
         assert(fma_ki.visited);

         if (m_usable_threads > 0) {
            cost = fma_jk.cost + fma_ki.cost;
         } else {
            cost = std::max(fma_jk.cost, fma_ki.cost);
         }
      }

      // Perform fma_jk and fma_ki in prallel
      if (t > 1) {
         for (std::size_t t1 = 1; t1 < t; ++t1) {
            const std::size_t t2 = t - t1;
            const DPNode& fma_jk = node(j, k + 1, t1);
            const DPNode& fma_ki = node(k, i, t2);
            assert(fma_jk.visited);
            assert(fma_ki.visited);

            const std::size_t c = std::max(fma_jk.cost, fma_ki.cost);
            if (c < cost) {
               cost = c;
               thread_split = t1;
            }
         }
      }

      // Dense
      const std::size_t fma = m_chain.elemental_jacobians[j].m *
                              m_chain.elemental_jacobians[k].m *
                              m_chain.elemental_jacobians[i].n;
      cost += fma;

      DPNode& fma_ji = node(j, i, t);
      if (cost < fma_ji.cost) {
         fma_ji.op.action = Action::MULTIPLICATION;
         fma_ji.op.mode = Mode::NONE;
         fma_ji.op.fma = fma;
         fma_ji.op.i = i;
         fma_ji.op.j = j;
         fma_ji.op.k = k;
         fma_ji.cost = cost;
         fma_ji.thread_split = thread_split;
         fma_ji.visited = true;
      }
   }

   template<Mode mode>
   auto try_elimination(
        const std::size_t j, const std::size_t i, const std::size_t k,
        const std::size_t t) -> void {
      std::size_t cost;
      std::size_t fma;
      if constexpr (mode == Mode::ADJOINT) {
         if (m_available_memory > 0) {
            const std::size_t mem = m_chain.get_jacobian(k, i).edges_in_dag;
            if (mem > m_available_memory) {
               return;
            }
         }

         const DPNode& fma_jk = node(j, k + 1, t);
         assert(fma_jk.visited);

         fma = m_chain.get_jacobian(k, i).fma<mode>(
              m_chain.elemental_jacobians[j].m);
         cost = fma_jk.cost + fma;
      } else {
         const DPNode& fma_ki = node(k, i, t);
         assert(fma_ki.visited);

         fma = m_chain.get_jacobian(j, k + 1).fma<mode>(
              m_chain.elemental_jacobians[i].n);
         cost = fma_ki.cost + fma;
      }

      DPNode& fma_ji = node(j, i, t);
      if (cost < fma_ji.cost) {
         fma_ji.op.action = Action::ELIMINATION;
         fma_ji.op.mode = mode;
         fma_ji.op.fma = fma;
         fma_ji.op.i = i;
         fma_ji.op.j = j;
         fma_ji.op.k = k;
         fma_ji.cost = cost;
         fma_ji.thread_split = 1;
         fma_ji.visited = true;
      }
   }
};

}  // namespace jcdp::optimizer

#endif  // JCDP_OPTIMIZER_DYNAMIC_PROGRAMMING_HPP_
