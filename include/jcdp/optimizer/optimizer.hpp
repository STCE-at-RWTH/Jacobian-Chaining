/******************************************************************************
 * @file jcdp/optimizer/optimizer.hpp
 *
 * @brief This file is part of the JCDP package. It provides a base class
 *        for an optimizer that finds tries to find the best possible
 *        brackating (elimination sequence) for a given Jacobian chain.
 ******************************************************************************/

#ifndef JCDP_OPTIMIZER_OPTIMIZER_HPP_
#define JCDP_OPTIMIZER_OPTIMIZER_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

#include "jcdp/jacobian_chain.hpp"
#include "jcdp/util/properties.hpp"

namespace jcdp {
class Sequence;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

namespace jcdp::optimizer {

class Optimizer : public Properties {
 public:
   Optimizer() {
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

   virtual ~Optimizer() = default;

   virtual auto init(const JacobianChain& chain) -> void {
      m_length = chain.length();
      m_usable_threads = std::min(m_available_threads, m_length);

      m_chain = chain;
      m_chain.optimized_costs.clear();
      m_chain.optimized_costs.resize(1 + m_usable_threads);
   }

   virtual auto solve() -> Sequence = 0;

   std::size_t m_usable_threads {0};

 protected:
   std::size_t m_length {0};
   bool m_matrix_free {false};
   bool m_banded {false};
   bool m_sparse {false};
   std::size_t m_available_memory {0};
   std::size_t m_available_threads {0};

   JacobianChain m_chain;
};

}  // end namespace jcdp::optimizer

#endif  // JCDP_OPTIMIZER_OPTIMIZER_HPP_
