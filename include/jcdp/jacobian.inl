#ifndef JCDP_INCLUDE_JACOBIAN_INL_
#define JCDP_INCLUDE_JACOBIAN_INL_

// IWYU pragma: private; include "util/jacobian.hpp"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <cmath>
#include <cstddef>
#include <list>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>

#include "jcdp/jacobian.hpp"

// >>>>>>>>>>>>>>>>>>>>>> TEMPLATE AND INLINE CONTENTS <<<<<<<<<<<<<<<<<<<<<< //

namespace jcdp {

// ------------------------------- Jacobian --------------------------------- //

template<class Generator, class IntDistribution, class RealDistribution>
inline auto Jacobian::generate_random(
     Generator& gen, IntDistribution& size_distribution,
     IntDistribution& cost_distribution, RealDistribution& mem_distribution,
     RealDistribution& density_distribution, const std::optional<std::size_t> n)
     -> Jacobian {

   Jacobian jac;
   jac.n = n.or_else([&]() -> std::optional<std::size_t> {
               return size_distribution(gen);
            }).value();
   jac.m = size_distribution(gen);

   jac.kl = std::round((jac.m - 1) * density_distribution(gen));
   jac.ku = std::round((jac.n - 1) * density_distribution(gen));

   const std::size_t max_mn = std::max(jac.m, jac.n);
   jac.non_zero_elements = max_mn;
   jac.non_zero_elements += std::round(
        (max_mn - jac.m * jac.n) * density_distribution(gen));

   jac.tangent_fma = cost_distribution(gen);
   jac.adjoint_fma = cost_distribution(gen);
   jac.adjoint_persistent_memory = static_cast<std::size_t>(
        jac.adjoint_fma * mem_distribution(gen));

   return jac;
}

}  // namespace jcdp

#endif  // JCDP_INCLUDE_JACOBIAN_INL_
