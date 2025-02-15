#ifndef JCDP_OPTIMIZER_BRANCH_AND_BOUND_HPP_
#define JCDP_OPTIMIZER_BRANCH_AND_BOUND_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <omp.h>

#include <cstddef>
#include <print>
#include <vector>
#include <utility>

#include "jcdp/jacobian.hpp"
#include "jcdp/jacobian_chain.hpp"
#include "jcdp/operation.hpp"
#include "jcdp/optimizer/optimizer.hpp"
#include "jcdp/sequence.hpp"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

namespace jcdp::optimizer {

class BranchAndBoundOptimizer : public Optimizer {

   using OpPair = std::array<std::optional<Operation>, 2>;

 public:
   virtual auto solve() -> Sequence override final {

      std::size_t accs = m_matrix_free ? 0 : (m_length - 1);
      m_leafs = 0;
      m_updated_makespan = 0;
      m_pruned_branches.clear();
      m_pruned_branches.resize(m_chain.longest_possible_sequence() + 1);

      #pragma omp parallel default(shared)
      #pragma omp single
      while (++accs <= m_length) {
         Sequence sequence {};
         std::vector<OpPair> eliminations {};
         JacobianChain chain = m_chain;
         add_accumulation(sequence, chain, accs, eliminations);
      }

      return m_optimal_sequence;
   }

   inline auto set_upper_bound(const std::size_t upper_bound) {
      m_makespan = upper_bound + 1;
   }

   inline auto print_stats() -> void {
      std::println("Number of leafs: {}", m_leafs);
      std::println("Updated makespan: {}", m_updated_makespan);
      std::println("Pruned branches per sequence length:");
      for (const std::size_t pruned : m_pruned_branches) {
         std::print("{} ", pruned);
      }
   }

 private:
   Sequence m_optimal_sequence {Sequence::make_max()};
   std::size_t m_makespan {m_optimal_sequence.makespan()};
   std::size_t m_leafs {0};
   std::vector<std::size_t> m_pruned_branches {};
   std::size_t m_updated_makespan {0};

   inline auto add_accumulation(
        Sequence sequence, JacobianChain chain, const std::size_t accs,
        std::vector<OpPair> eliminations, std::size_t j = 0) -> void {
      if (accs > 0) {
         for (; j < m_chain.length(); ++j) {
            const Operation op = cheapest_accumulation(j);
            if (!chain.apply(op)) {
               continue;
            }

            push_possible_eliminations(chain, eliminations, op.j, op.i);
            sequence.push_back(std::move(op));

            add_accumulation(sequence, chain, accs - 1, eliminations, j + 1);

            sequence.pop_back();
            eliminations.pop_back();
            chain.revert(op);
         }
      } else {
         #pragma omp task default(shared)                                      \
                          firstprivate(sequence, chain, eliminations)
         {
            add_elimination(sequence, chain, eliminations);
         }
      }
   }

   inline auto add_elimination(
        Sequence &sequence, JacobianChain &chain,
        std::vector<OpPair> &eliminations, std::size_t elim_idx = 0)
        -> void {

      // Check if we accumulated the entire jacobian
      if (chain.get_jacobian(chain.length() - 1, 0).is_accumulated) {
         assert(elim_idx == eliminations.size() - 1);
         assert(!eliminations[elim_idx][0].has_value());
         assert(!eliminations[elim_idx][1].has_value());

         Sequence final_sequence = sequence;

         // Start new task for the scheduling of the final sequence. If
         // branch & bound is used, this can take some time.
         #pragma omp task default(shared) firstprivate(final_sequence) untied
         {
            const std::size_t new_makespan = m_scheduler->schedule(
               final_sequence, m_usable_threads, m_makespan);

            #pragma omp atomic
            m_leafs++;

            #pragma omp critical
            if (m_makespan > new_makespan) {
               m_optimal_sequence = final_sequence;
               m_makespan = new_makespan;
               m_updated_makespan++;
            }
         }
         return;
      } else {
         bool prune = (sequence.critical_path() >= m_makespan);
         // if (!prune) {
         //    const std::size_t new_makespan = m_scheduler->schedule(
         //      sequence, m_usable_threads, m_makespan);

         //    prune = (new_makespan>= m_makespan);
         // }

         if (prune) {
            std::size_t &prune_counter = m_pruned_branches[sequence.length()];

            #pragma omp atomic
            prune_counter++;

            return;
         }
      }

      for (; elim_idx < eliminations.size(); ++elim_idx) {
         for (std::size_t pair_idx = 0; pair_idx <= 1; ++pair_idx) {
            if (!eliminations[elim_idx][pair_idx].has_value()) {
               continue;
            }

            const Operation op = eliminations[elim_idx][pair_idx].value();
            if (!chain.apply(op)) {
               continue;
            }

            push_possible_eliminations(chain, eliminations, op.j, op.i);
            sequence.push_back(op);

            add_elimination(sequence, chain, eliminations, elim_idx + 1);

            sequence.pop_back();
            eliminations.pop_back();
            chain.revert(op);
         }
      }
   }

   inline auto cheapest_accumulation(const std::size_t j) -> Operation {

      const Jacobian& jac = m_chain.get_jacobian(j, j);
      Operation op {
           .action = Action::ACCUMULATION,
           .mode = Mode::TANGENT,
           .j = j,
           .k = j,
           .i = j,
           .fma = jac.fma<Mode::TANGENT>()};

      if (m_available_memory == 0 || m_available_memory >= jac.edges_in_dag) {
         const std::size_t adjoint_fma = jac.fma<Mode::ADJOINT>();
         if (adjoint_fma < op.fma) {
            op.mode = Mode::ADJOINT;
            op.fma = adjoint_fma;
         }
      }

      return op;
   }

   inline auto push_possible_eliminations(
        const JacobianChain& chain, std::vector<OpPair>& eliminations,
        const std::size_t op_j, const std::size_t op_i) -> void {

      OpPair ops{};

      // Tangent or multiplication
      if (op_j < chain.length() - 1) {
         const std::size_t k = op_j;
         const std::size_t i = op_i;
         const Jacobian& ki_jac = chain.get_jacobian(k, i);

         // Add multiplication if possible
         std::size_t j;
         for (j = m_chain.length() - 1; j >= k + 1; --j) {
            const Jacobian& jk_jac = chain.get_jacobian(j, k + 1);
            if (!jk_jac.is_accumulated || jk_jac.is_used) {
               continue;
            }

            ops[0] = Operation {
                 .action = Action::MULTIPLICATION,
                 .j = j,
                 .k = k,
                 .i = i,
                 .fma = jk_jac.m * ki_jac.m * ki_jac.n};

            break;
         }

         // Add tangent elimination if multiplication wasn't possible
         if (k + 1 == ++j && m_matrix_free) {
            const Jacobian& jk_jac = chain.get_jacobian(j, k + 1);
            assert(!jk_jac.is_accumulated && !jk_jac.is_used);

            ops[0] = Operation {
                 .action = Action::ELIMINATION,
                 .mode = Mode::TANGENT,
                 .j = j,
                 .k = k,
                 .i = i,
                 .fma = jk_jac.fma<Mode::TANGENT>(ki_jac.n)};
         }
      }

      // Adjoint or multiplication
      if (op_i > 0) {
         const std::size_t k = op_i - 1;
         const std::size_t j = op_j;
         const Jacobian& jk_jac = chain.get_jacobian(j, k + 1);

         // Add multiplication if possible
         std::size_t i;
         for (i = 0; i <= k; ++i) {
            const Jacobian& ki_jac = chain.get_jacobian(k, i);
            if (!ki_jac.is_accumulated || ki_jac.is_used) {
               continue;
            }

            ops[1] = Operation {
                 .action = Action::MULTIPLICATION,
                 .j = j,
                 .k = k,
                 .i = i,
                 .fma = jk_jac.m * ki_jac.m * ki_jac.n};
            break;
         }

         // Add adjoint elimination if multiplication wasn't possible
         if (k == --i && m_matrix_free) {
            const Jacobian& ki_jac = chain.get_jacobian(k, i);
            assert(!ki_jac.is_accumulated && !ki_jac.is_used);

            if (m_available_memory == 0 ||
                m_available_memory >= ki_jac.edges_in_dag) {
               ops[1] = Operation {
                    .action = Action::ELIMINATION,
                    .mode = Mode::ADJOINT,
                    .j = j,
                    .k = k,
                    .i = i,
                    .fma = ki_jac.fma<Mode::ADJOINT>(jk_jac.m)};
            }
         }
      }

      eliminations.push_back(ops);
   }
};

}  // namespace jcdp::optimizer

#endif  // JCDP_OPTIMIZER_BRANCH_AND_BOUND_HPP_
