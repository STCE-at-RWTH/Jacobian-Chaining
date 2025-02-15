/******************************************************************************
 * @file jcdp/generator.hpp
 *
 * @brief This file is part of the JCDP package. It provides a Jacobian
 *        generator that produces random chains of Jacobians based on given
 *        properties.
 ******************************************************************************/

#ifndef JCDP_GENERATOR_HPP_
#define JCDP_GENERATOR_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "jcdp/jacobian.hpp"
#include "jcdp/jacobian_chain.hpp"
#include "jcdp/util/properties.hpp"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

namespace jcdp {

class JacobianChainGenerator : public Properties {
 public:
   JacobianChainGenerator() {
      register_property(
           m_chain_lengths, "length", "Lengths of the Jacobian Chains.");
      register_property(
           m_amount, "amount",
           "Amount of random Jacobian Chains (per length).");
      register_property(
           m_size_range, "size_range", "Range of the Jacobian dimensions.");
      register_property(
           m_dag_size_range, "dag_size_range",
           "Range of the amount of edges in the DAG of a single function F.");
      register_property(
           m_tangent_factor_range, "tangent_factor_range",
           "Range of the tangent runtime factor.");
      register_property(
           m_adjoint_factor_range, "adjoint_factor_range",
           "Range of the adjoint runtime factor.");
      register_property(
           m_density_range, "density_range",
           "Range of density percentages of the Jacobians. Used to calcluate "
           "number of non-zero entries and bandwidth.");
      register_property(
           m_seed, "seed", "Seed for the random number generator.");
   }

   inline auto init_rng() -> void {
      m_gen.seed(m_seed);
      m_size_distribution.param(int_bounds(m_size_range));
      m_dag_size_distribution.param(int_bounds(m_dag_size_range));
      m_tangent_factor_distribution.param(real_bounds(m_tangent_factor_range));
      m_adjoint_factor_distribution.param(real_bounds(m_adjoint_factor_range));
      m_density_distribution.param(real_bounds(m_density_range));
   }

   //! Generate a random Jacobian chain.
   inline auto next(JacobianChain& chain) -> bool {
      const std::size_t idx = length_idx * m_amount + batch_idx;
      if (idx >= m_amount * m_chain_lengths.size()) {
         return false;
      }

      chain.elemental_jacobians.clear();
      chain.elemental_jacobians.reserve(m_chain_lengths[length_idx]);
      chain.elemental_jacobians.push_back(generate_random_jacobian());
      chain.elemental_jacobians[0].i = 0;
      chain.elemental_jacobians[0].j = 1;

      for (std::size_t i = 1; i < m_chain_lengths[length_idx]; ++i) {
         chain.elemental_jacobians.push_back(
              generate_random_jacobian(chain.elemental_jacobians[i - 1].m));
         chain.elemental_jacobians[i].i = i;
         chain.elemental_jacobians[i].j = i + 1;
      }

      chain.id = batch_idx;

      if (++batch_idx >= m_amount) {
         batch_idx = 0;
         length_idx++;
      }
      return true;
   }

 private:
   // Chain properties
   std::vector<std::size_t> m_chain_lengths {1};
   std::size_t m_amount {1};
   std::pair<std::size_t, std::size_t> m_size_range {1, 1};
   std::pair<std::size_t, std::size_t> m_dag_size_range {1, 1};
   std::pair<double, double> m_tangent_factor_range {1.0, 1.0};
   std::pair<double, double> m_adjoint_factor_range {1.0, 1.0};
   std::pair<double, double> m_density_range {0.0, 1.0};
   std::size_t m_seed {[]() -> std::size_t {
      std::random_device rd;
      return rd();
   }()};

   // Internal RNG state
   std::mt19937_64 m_gen;
   std::uniform_int_distribution<std::size_t> m_size_distribution;
   std::uniform_int_distribution<std::size_t> m_dag_size_distribution;
   std::uniform_real_distribution<double> m_tangent_factor_distribution;
   std::uniform_real_distribution<double> m_adjoint_factor_distribution;
   std::uniform_real_distribution<double> m_density_distribution;

   std::size_t batch_idx {0};
   std::size_t length_idx {0};

   using int_param_t = std::uniform_int_distribution<std::size_t>::param_type;
   using real_param_t = std::uniform_real_distribution<double>::param_type;

   inline auto int_bounds(const std::pair<std::size_t, std::size_t>& b)
        -> int_param_t {
      return int_param_t {b.first, b.second};
   }

   inline auto real_bounds(const std::pair<double, double>& b) -> real_param_t {
      return real_param_t {b.first, b.second};
   }

   inline auto generate_random_jacobian(const std::optional<std::size_t> n = {})
        -> Jacobian {
      Jacobian jac;
      jac.n = n.value_or(m_size_distribution(m_gen));
      jac.m = m_size_distribution(m_gen);

      jac.kl = static_cast<std::size_t>(
           std::round((jac.m - 1) * m_density_distribution(m_gen)));
      jac.ku = static_cast<std::size_t>(
           std::round((jac.n - 1) * m_density_distribution(m_gen)));

      const std::size_t max_mn = std::max(jac.m, jac.n);
      jac.non_zero_elements = max_mn;
      jac.non_zero_elements += static_cast<std::size_t>(std::round(
           (max_mn - jac.m * jac.n) * m_density_distribution(m_gen)));

      jac.edges_in_dag = m_dag_size_distribution(m_gen);
      jac.tangent_cost = static_cast<std::size_t>(
           std::round(jac.edges_in_dag * m_tangent_factor_distribution(m_gen)));
      jac.adjoint_cost = static_cast<std::size_t>(
           std::round(jac.edges_in_dag * m_adjoint_factor_distribution(m_gen)));

      return jac;
   }
};

}  // end namespace jcdp

#endif  // JCDP_GENERATOR_HPP_
