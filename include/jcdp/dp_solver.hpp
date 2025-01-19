#ifndef JCDP_DP_SOLVER_HPP_
#define JCDP_DP_SOLVER_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <optional>
#include <print>
#include <random>
#include <vector>

#include "jcdp/jacobian.hpp"
#include "jcdp/jacobian_chain.hpp"
#include "jcdp/operation.hpp"
#include "jcdp/util/properties.hpp"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

namespace jcdp {

struct DPNode {
   std::size_t cost{std::numeric_limits<std::size_t>::max()};
   std::size_t position_split{0};
   std::size_t thread_split{0};
   Operation op{Operation::NONE};
   Mode mode{Mode::NONE};
   bool visited{false};
};

class DPSolver : public Properties {
 public:
   DPSolver() {
      register_property(
           m_matrix_free, "matrix_free",
           "Wether we optimize the matrix-free problem.");
      register_property(
           m_banded, "banded",
           "Wether the assume that the Jacobians are banded.");
      register_property(
           m_sparse, "sparse",
           "Wether the assume that the Jacobians are sparse.");
      register_property(
           m_available_memory, "available_memory",
           "Amount of available persistent memory.");
      register_property(
           m_available_threads, "available_threads",
           "Amount of threads that are available for the evaluation of the "
           "jacobian chain.");
   }

   auto init(JacobianChain&& chain) -> void {
      m_chain = chain;
      m_length = chain.jacobians.size();

      std::size_t dp_nodes = m_length * m_length;
      if (m_available_threads >= m_length || m_available_threads < 1) {
         m_available_threads = 0;
      } else {
         dp_nodes *= m_available_threads;
      }

      m_dptable.resize(dp_nodes);
   }

   auto solve() -> void {

      std::size_t threads = 1;
      do {
         for (std::size_t j = 0; j < m_length; ++j) {
            try_accumulation<Mode::TANGENT>(j, threads);
            try_accumulation<Mode::ADJOINT>(j, threads);
         }

         for (std::size_t len = 2; len <= m_length; ++len) {
            for (std::size_t j = len - 1; j < m_length; ++j) {
               const std::size_t i = j - (len - 1);

               for (std::size_t k = i; k < j; k++) {
                  try_multiplication(j, i, k, threads);

                  if (m_matrix_free) {
                     try_elimination<Mode::TANGENT>(j, i, k, threads);
                     try_elimination<Mode::ADJOINT>(j, i, k, threads);
                  }
               }
            }
         }
      } while (++threads <= m_available_threads);
   }

   auto print_sequence() -> void {
      print_operations(m_length - 1, 0, {0, m_available_threads - 1});
   }

   auto print_operations(
        const std::size_t j, const std::size_t i,
        const std::pair<std::size_t, std::size_t> thread_pool) -> void {

      const std::size_t t = thread_pool.second - thread_pool.first + 1;
      DPNode& fma_ji = node(j, i, t);
      assert(fma_ji.visited);

      switch (fma_ji.op) {
         case Operation::ACCUMULATION: {
            std::print(
                 "ACC {} ({} {})\t", to_string(fma_ji.mode), j, i,
                 thread_pool.first);
         } break;
         case Operation::MULTIPLICATION: {
            std::pair<std::size_t, std::size_t> thread_pool_jk = thread_pool;
            std::pair<std::size_t, std::size_t> thread_pool_ki = thread_pool;
            if (fma_ji.thread_split > 0) {
               thread_pool_ki.first = thread_pool.first + fma_ji.thread_split;
               thread_pool_jk.second = thread_pool_ki.first - 1;
            }
            print_operations(j, fma_ji.position_split + 1, thread_pool_jk);
            print_operations(fma_ji.position_split, i, thread_pool_ki);

            std::print(
                 "ELI MUL ({} {}) ({} {})", j, fma_ji.position_split + 1,
                 fma_ji.position_split, i);
         } break;
         case Operation::ELIMINATION: {
            if (fma_ji.mode == Mode::TANGENT) {
               print_operations(fma_ji.position_split, i, thread_pool);
            } else if (fma_ji.mode == Mode::ADJOINT) {
               print_operations(j, fma_ji.position_split + 1, thread_pool);
            }

            std::print(
                 "ELI {} ({} {} {})\t", to_string(fma_ji.mode), j,
                 fma_ji.position_split, i);
         } break;
         case Operation::NONE: {
            assert(false);
         } break;
      }

      if (thread_pool.first == thread_pool.second) {
         std::print("\t[{}]", thread_pool.first);
      } else {
         std::print("\t[{}-{}]", thread_pool.first, thread_pool.second);
      }

      std::println("\t{}", fma_ji.cost);
   }

 private:
   std::size_t m_length{0};
   bool m_matrix_free{false};
   bool m_banded{false};
   bool m_sparse{false};
   std::size_t m_available_memory{0};
   std::size_t m_available_threads{0};
   JacobianChain m_chain;

   std::vector<DPNode> m_dptable;

   auto node(const std::size_t j, const std::size_t i, const std::size_t t)
        -> DPNode& {
      std::size_t idx = j * m_length + i;
      if (m_available_threads > 0) {
         idx += (t - 1) * m_length * m_length;
      }
      return m_dptable[idx];
   }

   template<Mode mode>
   auto try_accumulation(const std::size_t j, const std::size_t t) -> void {
      if constexpr (mode == Mode::ADJOINT) {
         if (m_chain.subchain_memory_requirement(j, j) > m_available_memory) {
            return;
         }
      }

      std::size_t cost = m_chain.subchain_fma<mode>(j, j);
      DPNode& fma_ji = node(j, j, t);
      if (cost < fma_ji.cost) {
         fma_ji.op = Operation::ACCUMULATION;
         fma_ji.position_split = j;
         fma_ji.thread_split = 0;
         fma_ji.mode = mode;
         fma_ji.cost = cost;
         fma_ji.visited = true;
      }
   }

   auto try_multiplication(
        const std::size_t j, const std::size_t i, const std::size_t k,
        const std::size_t t) -> void {

      std::size_t cost = std::numeric_limits<std::size_t>::max();
      std::size_t thread_split = 0;

      if (t == 1 && m_available_threads > 0) {
         const DPNode& fma_jk = node(j, k + 1, t);
         const DPNode& fma_ki = node(k, i, t);
         assert(fma_jk.visited);
         assert(fma_ki.visited);

         cost = fma_jk.cost + fma_ki.cost;
      } else {
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
      cost += m_chain.jacobians[j].m * m_chain.jacobians[k].m *
              m_chain.jacobians[i].n;

      DPNode& fma_ji = node(j, i, t);
      if (cost < fma_ji.cost) {
         fma_ji.op = Operation::MULTIPLICATION;
         fma_ji.position_split = k;
         fma_ji.thread_split = thread_split;
         fma_ji.mode = Mode::NONE;
         fma_ji.cost = cost;
         fma_ji.visited = true;
      }
   }

   template<Mode mode>
   auto try_elimination(
        const std::size_t j, const std::size_t i, const std::size_t k,
        const std::size_t t) -> void {

      std::size_t cost;
      if constexpr (mode == Mode::ADJOINT) {
         if (m_chain.subchain_memory_requirement(k, i) > m_available_memory) {
            return;
         }

         const DPNode& fma_jk = node(j, k + 1, t);
         assert(fma_jk.visited);
         cost = fma_jk.cost + m_chain.subchain_fma<mode>(k, i);
      } else {
         const DPNode& fma_ki = node(k, i, t);
         assert(fma_ki.visited);
         cost = fma_ki.cost + m_chain.subchain_fma<mode>(j, k + 1);
      }

      DPNode& fma_ji = node(j, i, t);
      if (cost < fma_ji.cost) {
         fma_ji.op = Operation::ELIMINATION;
         fma_ji.position_split = k;
         fma_ji.thread_split = 1;
         fma_ji.mode = mode;
         fma_ji.cost = cost;
      }
   }
};

}  // end namespace jcdp

// >>>>>>>>>>>>>>>> INCLUDE TEMPLATE AND INLINE DEFINITIONS <<<<<<<<<<<<<<<<< //

// #include "util/impl/jacobian.inl"  // IWYU pragma: export

#endif  // JCDP_DP_SOLVER_HPP_
