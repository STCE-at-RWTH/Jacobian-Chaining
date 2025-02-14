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
   std::vector<Jacobian> elemental_jacobians {};
   std::vector<Jacobian> sub_chains {};
   std::vector<std::size_t> optimized_costs {};
   std::size_t id {0};

   inline auto length() const -> std::size_t {
      return elemental_jacobians.size();
   }

   inline auto init_subchains() -> void {
      const std::size_t len = length();
      sub_chains.resize(len * (len - 1) / 2);

      for (std::size_t j = 0; j < len; ++j) {
         for (std::size_t i = 0; i < j; ++i) {
            const std::size_t idx = j * (j - 1) / 2 + i;

            sub_chains[idx].i = elemental_jacobians[i].i;
            sub_chains[idx].j = elemental_jacobians[j].j;
            sub_chains[idx].n = elemental_jacobians[i].n;
            sub_chains[idx].m = elemental_jacobians[j].m;

            for (std::size_t k = i; k <= j; ++k) {
               sub_chains[idx].edges_in_dag +=
                    elemental_jacobians[k].edges_in_dag;
               sub_chains[idx].tangent_cost +=
                    elemental_jacobians[k].tangent_cost;
               sub_chains[idx].adjoint_cost +=
                    elemental_jacobians[k].adjoint_cost;
            }
         }
      }
   }

   inline auto apply(const Operation& op) -> bool {
      bool& ij_is_acc = get_jacobian(op.j, op.i).is_accumulated;
      if (ij_is_acc) {
         return false;
      }

      if (op.action != Action::ACCUMULATION) {
         bool& jk_is_acc = get_jacobian(op.j, op.k + 1).is_accumulated;
         bool& ki_is_acc = get_jacobian(op.k, op.i).is_accumulated;

         switch (op.mode) {
            case Mode::TANGENT: {
               if (!ki_is_acc || jk_is_acc) {
                  return false;
               }
               ki_is_acc = false;
            } break;

            case Mode::ADJOINT: {
               if (!jk_is_acc || ki_is_acc) {
                  return false;
               }
               jk_is_acc = false;
            } break;

            case Mode::NONE: {
               if (!jk_is_acc || !ki_is_acc) {
                  return false;
               }
               ki_is_acc = false;
               jk_is_acc = false;
            } break;
         }
      }

      ij_is_acc = true;
      return true;
   }

   inline auto revert(const Operation& op) {
      Jacobian& ij_jac = get_jacobian(op.j, op.i);
      assert(ij_jac.is_accumulated);
      ij_jac.is_accumulated = false;

      if (op.action != Action::ACCUMULATION) {
         if (op.mode != Mode::TANGENT) {
            Jacobian& jk_jac = get_jacobian(op.j, op.k + 1);
            assert(!jk_jac.is_accumulated);
            jk_jac.is_accumulated = true;
         }

         if (op.mode != Mode::ADJOINT) {
            Jacobian& ki_jac = get_jacobian(op.k, op.i);
            assert(!ki_jac.is_accumulated);
            ki_jac.is_accumulated = true;
         }
      }
   }

 private:
   template<typename Self>
   inline static auto get_jacobian_impl(
        Self& self, const std::size_t j, const std::size_t i)
        -> decltype(auto) {
      assert(j < self.elemental_jacobians.size());
      assert(i < self.elemental_jacobians.size());
      assert(j >= i);

      if (j == i) {
         return self.elemental_jacobians[j];
      }

      const std::size_t idx = j * (j - 1) / 2 + i;
      assert(self.sub_chains.size() > idx);
      return self.sub_chains[idx];
   }

 public:
   inline auto get_jacobian(const std::size_t j, const std::size_t i)
        -> Jacobian& {
      return get_jacobian_impl(*this, j, i);
   }

   inline auto get_jacobian(const std::size_t j, const std::size_t i) const
        -> const Jacobian& {
      return get_jacobian_impl(*this, j, i);
   }
};

}  // end namespace jcdp

// >>>>>>>>>>>>>>>> INCLUDE TEMPLATE AND INLINE DEFINITIONS <<<<<<<<<<<<<<<<< //

// #include "util/impl/jacobian.inl"  // IWYU pragma: export

#endif  // JCDP_JACOBIAN_CHAIN_HPP_
