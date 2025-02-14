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
   //! Cost of a single tangent evaluation (y^(1) = F' * x^(1)).
   std::size_t tangent_cost {0};
   //! Cost of a single adjoint evaluation (x_(1) = y_(1) * F').
   std::size_t adjoint_cost {0};

   //! Whether the Jacobian is already accumulated or not.
   bool is_accumulated {false};

   //! Whether the Jacobian is already used in an elimination.
   bool is_used {false};

   template<Mode mode>
   inline auto fma(const std::optional<std::size_t> evals = {}) const
        -> std::size_t {
      if constexpr (mode == Mode::ADJOINT) {
         return evals.value_or(m) * adjoint_cost;
      } else {
         return evals.value_or(n) * tangent_cost;
      }
   }
};

}  // end namespace jcdp

// >>>>>>>>>>>>>>>> INCLUDE TEMPLATE AND INLINE DEFINITIONS <<<<<<<<<<<<<<<<< //

// #include "jcdp/jacobian.inl"  // IWYU pragma: export

#endif  // JCDP_JACOBIAN_HPP_
