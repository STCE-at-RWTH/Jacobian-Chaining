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
     IntDistribution& dag_size_distribution,
     RealDistribution& tangent_factor_distribution,
     RealDistribution& adjoint_factor_distribution,
     RealDistribution& density_distribution, const std::optional<std::size_t> n)
     -> Jacobian {

   Jacobian jac;
   jac.n = n.or_else([&]() -> std::optional<std::size_t> {
               return size_distribution(gen);
            }).value();
   jac.m = size_distribution(gen);

   jac.kl = static_cast<std::size_t>(
        std::round((jac.m - 1) * density_distribution(gen)));
   jac.ku = static_cast<std::size_t>(
        std::round((jac.n - 1) * density_distribution(gen)));

   const std::size_t max_mn = std::max(jac.m, jac.n);
   jac.non_zero_elements = max_mn;
   jac.non_zero_elements += static_cast<std::size_t>(
        std::round((max_mn - jac.m * jac.n) * density_distribution(gen)));

   jac.edges_in_dag = dag_size_distribution(gen);
   jac.tangent_factor = tangent_factor_distribution(gen);
   jac.adjoint_factor = adjoint_factor_distribution(gen);

   return jac;
}

}  // namespace jcdp

#endif  // JCDP_INCLUDE_JACOBIAN_INL_
