#ifndef JCDP_JACOBIAN_CHAIN_HPP_
#define JCDP_JACOBIAN_CHAIN_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <cassert>
#include <cstddef>
#include <vector>

#include "jcdp/jacobian.hpp"
#include "jcdp/operation.hpp"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

namespace jcdp {

struct JacobianChain {
   std::vector<Jacobian> jacobians;
   std::vector<std::size_t> optimized_costs;
   std::size_t id;

   inline auto length() -> std::size_t {
      return jacobians.size();
   }

   template<Mode mode>
   inline auto subchain_fma(
        const std::size_t j, const std::size_t i, const std::size_t seed) const
        -> std::size_t {
      assert(j < jacobians.size() && i < jacobians.size() && j >= i);

      std::size_t single_eval_fma = 0;
      for (std::size_t idx = i; idx <= j; ++idx) {
         single_eval_fma += jacobians[idx].single_evaluation_fma<mode>();
      }

      if constexpr (mode == Mode::ADJOINT) {
         return jacobians[seed].m * single_eval_fma;
      } else {
         return jacobians[seed].n * single_eval_fma;
      }
   }

   inline auto subchain_memory_requirement(
        const std::size_t j, const std::size_t i) const -> std::size_t {
      assert(j < jacobians.size() && i < jacobians.size() && j >= i);

      std::size_t mem_req = 0;
      for (std::size_t idx = i; idx <= j; ++idx) {
         mem_req += jacobians[idx].edges_in_dag;
      }

      return mem_req;
   }
};

}  // end namespace jcdp

// >>>>>>>>>>>>>>>> INCLUDE TEMPLATE AND INLINE DEFINITIONS <<<<<<<<<<<<<<<<< //

// #include "util/impl/jacobian.inl"  // IWYU pragma: export

#endif  // JCDP_JACOBIAN_CHAIN_HPP_
