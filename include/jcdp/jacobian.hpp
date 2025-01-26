#ifndef JCDP_JACOBIAN_HPP_
#define JCDP_JACOBIAN_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <cstddef>
#include <optional>

#include "jcdp/operation.hpp"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

namespace jcdp {

struct Jacobian {
   //! Index of input variable.
   std::size_t i {0};
   //! Index of output variable.
   std::size_t j {0};

   //! Input size.
   std::size_t n {0};
   //! Output size.
   std::size_t m {0};

   //! Number of super-diagonals (upper bandwidth).
   std::size_t ku {0};
   //! Number of sub-diagonals (lower bandwidth).
   std::size_t kl {0};
   //! Number of non-zero elements (general sparsity).
   std::size_t non_zero_elements {0};

   //! Amount of edges in the DAG of the primal function (~ size of tape).
   std::size_t edges_in_dag {0};
   //! Runtime factor of a single tangent evaluation (y^(1) = F' * x^(1)).
   double tangent_factor {1};
   //! Runtime factor of a single adjoint evaluation (x_(1) = y_(1) * F').
   double adjoint_factor {1};

   //! Generate a random Jacobian matrix.
   template<class Generator, class IntDistribution, class RealDistribution>
   inline static auto generate_random(
        Generator& gen, IntDistribution& size_distribution,
        IntDistribution& dag_size_distribution,
        RealDistribution& tangent_factor_distribution,
        RealDistribution& adjoint_factor_distribution,
        RealDistribution& density_distribution,
        const std::optional<std::size_t> n = {}) -> Jacobian;


   template<Mode mode>
   inline auto single_evaluation_fma() const -> std::size_t {
      if constexpr (mode == Mode::ADJOINT) {
         return static_cast<std::size_t>(edges_in_dag * adjoint_factor);
      } else {
         return static_cast<std::size_t>(edges_in_dag * tangent_factor);
      }
   }
};

}  // end namespace jcdp

// >>>>>>>>>>>>>>>> INCLUDE TEMPLATE AND INLINE DEFINITIONS <<<<<<<<<<<<<<<<< //

// #include "jcdp/jacobian.inl"  // IWYU pragma: export

#endif  // JCDP_JACOBIAN_HPP_
