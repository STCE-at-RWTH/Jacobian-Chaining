#ifndef JCDP_JACOBIAN_CHAIN_HPP_
#define JCDP_JACOBIAN_CHAIN_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <cassert>
#include <cstddef>
#include <optional>
#include <vector>

#include "jcdp/jacobian.hpp"
#include "jcdp/operation.hpp"
#include "jcdp/sequence.hpp"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

namespace jcdp {

struct JacobianChain {
   std::vector<Jacobian> elemental_jacobians {};
   std::vector<std::optional<Jacobian>> sub_chains {};
   std::vector<std::size_t> optimized_costs {};
   std::size_t id {0};

   inline auto length() const -> std::size_t {
      return elemental_jacobians.size();
   }

   inline auto get_jacobian(const std::size_t j, const std::size_t i)
        -> const Jacobian& {
      assert(j < elemental_jacobians.size());
      assert(i < elemental_jacobians.size());
      assert(j >= i);

      if (j == i) {
         return elemental_jacobians[j];
      }

      const std::size_t idx = j * (j - 1) / 2 + i;
      if (sub_chains.size() > idx && sub_chains[idx].has_value()) {
         return sub_chains[idx].value();
      }

      #pragma omp critical
      {
         if (sub_chains.size() <= idx) {
            sub_chains.resize(idx + 1, {});
         } else if (sub_chains[idx].has_value()) {
            // Just in case two threads tried to calculate the same Jacobian
            return sub_chains[idx].value();
         }

         Jacobian chain {
              .i = elemental_jacobians[i].i,
              .j = elemental_jacobians[j].j,
              .n = elemental_jacobians[i].n,
              .m = elemental_jacobians[j].m,
         };

         for (std::size_t k = i; k <= j; ++k) {
            chain.edges_in_dag += elemental_jacobians[k].edges_in_dag;
            chain.tangent_cost += elemental_jacobians[k].tangent_cost;
            chain.adjoint_cost += elemental_jacobians[k].adjoint_cost;
         }

         sub_chains[idx] = chain;
      }

      return sub_chains[idx].value();
   }
};

}  // end namespace jcdp

// >>>>>>>>>>>>>>>> INCLUDE TEMPLATE AND INLINE DEFINITIONS <<<<<<<<<<<<<<<<< //

// #include "util/impl/jacobian.inl"  // IWYU pragma: export

#endif  // JCDP_JACOBIAN_CHAIN_HPP_
