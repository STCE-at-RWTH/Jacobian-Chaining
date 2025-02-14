#ifndef JCDP_OPTIMIZER_BRANCH_AND_BOUND_HPP_
#define JCDP_OPTIMIZER_BRANCH_AND_BOUND_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <omp.h>

#include <cstddef>
#include <print>

#include "jcdp/jacobian.hpp"
#include "jcdp/jacobian_chain.hpp"
#include "jcdp/operation.hpp"
#include "jcdp/optimizer/optimizer.hpp"
#include "jcdp/sequence.hpp"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

namespace jcdp::optimizer {

class BranchAndBoundOptimizer : public Optimizer {
 public:
   virtual auto solve() -> Sequence override final {

      std::size_t accs = m_matrix_free ? 0 : (m_length - 1);

      #pragma omp parallel default(shared)
      #pragma omp master
      while (++accs <= m_length) {
         Sequence sequence {};
         std::vector<Operation> eliminations {};
         JacobianChain chain = m_chain;
         add_accumulation(sequence, chain, accs, eliminations);
      }

      return m_optimal_sequence;
   }

   inline auto set_upper_bound(const std::size_t upper_bound) {
      m_makespan = upper_bound + 1;
   }

 private:
   Sequence m_optimal_sequence {Sequence::make_max()};
   std::size_t m_makespan {m_optimal_sequence.makespan()};

   inline auto add_accumulation(
        Sequence& sequence, JacobianChain& chain, const std::size_t accs,
        std::vector<Operation>& eliminations, std::size_t j = 0) -> void {
      if (accs > 0) {
         for (; j < m_chain.length(); ++j) {
            const Operation op = cheapest_accumulation(j);
            if (!chain.apply(op)) {
               continue;
            }

            sequence.push_back(std::move(op));
            const std::size_t new_eliminations = push_possible_eliminations(
                 chain, eliminations, op.j, op.i);

            add_accumulation(sequence, chain, accs - 1, eliminations, j + 1);

            pop_possible_eliminations(eliminations, new_eliminations);
            sequence.pop_back();
            chain.revert(op);
         }
      } else {
         #pragma omp task default(none)                                        \
                          firstprivate(sequence, chain, eliminations)
         add_elimination(sequence, chain, eliminations);
      }
   }

   inline auto add_elimination(
        Sequence& sequence, JacobianChain& chain,
        std::vector<Operation>& eliminations, std::size_t elim_idx = 0)
        -> void {

      // Check if we accumulated the entire jacobian
      if (chain.get_jacobian(chain.length() - 1, 0).is_accumulated) {
         assert(elim_idx == eliminations.size());
         const std::size_t new_makespan = m_scheduler->schedule(
              sequence, m_usable_threads, m_makespan);

         #pragma omp critical
         if (m_makespan > new_makespan) {
            m_optimal_sequence = sequence;
            m_makespan = new_makespan;
         }
         return;
      } else {
         // const std::size_t new_makespan = m_scheduler->schedule(
         //      sequence, m_usable_threads, m_makespan);

         if (sequence.critical_path() >= m_makespan) {
            return;
         }
      }

      for (; elim_idx < eliminations.size(); ++elim_idx) {
         const Operation& op = eliminations[elim_idx];
         if (!chain.apply(op)) {
            continue;
         }

         sequence.push_back(op);
         const std::size_t new_eliminations = push_possible_eliminations(
              chain, eliminations, op.j, op.i);

         add_elimination(sequence, chain, eliminations, elim_idx + 1);

         pop_possible_eliminations(eliminations, new_eliminations);
         sequence.pop_back();
         chain.revert(eliminations[elim_idx]);
      }
   }

   inline auto cheapest_accumulation(const std::size_t j) -> Operation {

      const Jacobian& jac = m_chain.get_jacobian(j, j);
      Operation op {
           .action = Action::ACCUMULATION,
           .i = j,
           .j = j,
           .k = j,
           .mode = Mode::TANGENT,
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
        const JacobianChain& chain, std::vector<Operation>& eliminations,
        const std::size_t op_j, const std::size_t op_i) -> std::size_t {

      size_t new_eliminations = 0;

      // Tangent or multiplication
      if (op_j < chain.length() - 1) {
         const std::size_t k = op_j;
         const std::size_t i = op_i;
         const Jacobian& ki_jac = chain.get_jacobian(k, i);

         // Add multiplication if possible
         std::size_t j;
         for (j = m_chain.length() - 1; j >= k + 1; --j) {
            const Jacobian& jk_jac = chain.get_jacobian(j, k + 1);
            if (!jk_jac.is_accumulated) {
               continue;
            }

            eliminations.push_back(Operation {
                 .action = Action::MULTIPLICATION,
                 .i = i,
                 .j = j,
                 .k = k,
                 .fma = jk_jac.m * ki_jac.m * ki_jac.n});

            new_eliminations++;
            break;
         }

         // Add tangent elimination if multiplication wasn't possible
         if (k + 1 > j++ && m_matrix_free) {
            const Jacobian& jk_jac = chain.get_jacobian(j, k + 1);
            assert(!jk_jac.is_accumulated);

            eliminations.push_back(Operation {
                 .action = Action::ELIMINATION,
                 .mode = Mode::TANGENT,
                 .i = i,
                 .j = j,
                 .k = k,
                 .fma = jk_jac.fma<Mode::TANGENT>(ki_jac.n)});

            new_eliminations++;
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
            if (!ki_jac.is_accumulated) {
               continue;
            }

            eliminations.push_back(Operation {
                 .action = Action::MULTIPLICATION,
                 .i = i,
                 .j = j,
                 .k = k,
                 .fma = jk_jac.m * ki_jac.m * ki_jac.n});

            new_eliminations++;
            break;
         }

         // Add adjoint elimination if multiplication wasn't possible
         if (k < i-- && m_matrix_free) {
            const Jacobian& ki_jac = chain.get_jacobian(k, i);
            assert(!ki_jac.is_accumulated);

            if (m_available_memory == 0 ||
                m_available_memory >= ki_jac.edges_in_dag) {
               eliminations.push_back(Operation {
                    .action = Action::ELIMINATION,
                    .mode = Mode::ADJOINT,
                    .i = i,
                    .j = j,
                    .k = k,
                    .fma = ki_jac.fma<Mode::ADJOINT>(jk_jac.m)});

               new_eliminations++;
            }
         }
      }

      return new_eliminations;
   }

   inline auto pop_possible_eliminations(
        std::vector<Operation>& eliminations, std::size_t amount) -> void {

      for (; amount-- > 0;) {
         eliminations.pop_back();
      }
   }
};

}  // namespace jcdp::optimizer

// >>>>>>>>>>>>>>>> INCLUDE TEMPLATE AND INLINE DEFINITIONS <<<<<<<<<<<<<<<<< //

// #include "util/impl/jacobian.inl"  // IWYU pragma: export

#endif  // JCDP_OPTIMIZER_BRANCH_AND_BOUND_HPP_
