#ifndef JCDP_JACOBIAN_CHAIN_HPP_
#define JCDP_JACOBIAN_CHAIN_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <cassert>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <random>
#include <vector>

#include "jcdp/jacobian.hpp"
#include "jcdp/operation.hpp"
#include "jcdp/util/properties.hpp"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

namespace jcdp {

class JacobianChainProperties : public Properties {
 public:
   size_t chain_length {1};
   std::pair<size_t, size_t> size_range {1, 1};
   std::pair<size_t, size_t> dag_size_range {1, 1};
   std::pair<double, double> tangent_factor_range {1.0, 1.0};
   std::pair<double, double> adjoint_factor_range {1.0, 1.0};
   std::pair<double, double> density_range {0.0, 1.0};
   size_t seed {[]() -> size_t {
      std::random_device rd;
      return rd();
   }()};

   JacobianChainProperties() {
      register_property(
           chain_length, "length", "Length of the Jacobian Chain.");
      register_property(
           size_range, "size_range", "Range of the Jacobian dimensions.");
      register_property(
           dag_size_range, "dag_size_range",
           "Range of the amount of edges in the DAG of a single function F.");
      register_property(
           tangent_factor_range, "tangent_factor_range",
           "Range of the tangent runtime factor.");
      register_property(
           adjoint_factor_range, "adjoint_factor_range",
           "Range of the adjoint runtime factor.");
      register_property(
           density_range, "density_range",
           "Range of density percentages of the Jacobians. Used to calcluate "
           "number of non-zero entries and bandwidth.");
      register_property(seed, "seed", "Seed for the RNG.");
   }
};

struct JacobianChain {
   std::vector<Jacobian> jacobians;
   std::size_t optimized_cost {std::numeric_limits<std::size_t>::max()};

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

   //! Generate a random Jacobian matrix.
   inline static auto generate_random(const JacobianChainProperties& p)
        -> JacobianChain {

      std::mt19937 gen(p.seed);
      std::uniform_int_distribution<std::size_t> size_distribution(
           p.size_range.first, p.size_range.second);
      std::uniform_int_distribution<std::size_t> dag_size_distribution(
           p.dag_size_range.first, p.dag_size_range.second);
      std::uniform_real_distribution<double> tangent_factor_distribution(
           p.tangent_factor_range.first, p.tangent_factor_range.second);
      std::uniform_real_distribution<double> adjoint_factor_distribution(
           p.adjoint_factor_range.first, p.adjoint_factor_range.second);
      std::uniform_real_distribution<double> density_distribution(
           p.density_range.first, p.density_range.second);

      JacobianChain chain;
      chain.jacobians.reserve(p.chain_length);
      chain.jacobians.push_back(Jacobian::generate_random(
           gen, size_distribution, dag_size_distribution,
           tangent_factor_distribution, adjoint_factor_distribution,
           density_distribution));
      chain.jacobians[0].i = 0;
      chain.jacobians[0].j = 1;

      for (std::size_t i = 1; i < p.chain_length; ++i) {
         chain.jacobians.push_back(Jacobian::generate_random(
              gen, size_distribution, dag_size_distribution,
              tangent_factor_distribution, adjoint_factor_distribution,
              density_distribution, chain.jacobians[i - 1].m));
         chain.jacobians[i].i = i;
         chain.jacobians[i].j = i + 1;
      }

      return chain;
   }

   //! Write the Jacobian chain into a graphml file.
   inline auto write_graphml(const std::filesystem::path& output_file) const
        -> void {
      std::ofstream file(output_file);
      if (file.is_open()) {
         file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
         file << "<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\" "
                 "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
                 "xsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns "
                 "http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd\">\n";
         file << "  <key id=\"fma_upper_bound\" for=\"graph\" "
                 "attr.name=\"fma_upper_bound\" attr.type=\"long\" />\n";
         file << "  <key id=\"index\" for=\"node\" "
                 "attr.name=\"index\" attr.type=\"long\" />\n";
         file << "  <key id=\"size\" for=\"node\" "
                 "attr.name=\"size\" attr.type=\"long\" />\n";
         file << "  <key id=\"adjoint_cost\" for=\"edge\" "
                 "attr.name=\"adjoint_cost\" attr.type=\"long\" />\n";
         file << "  <key id=\"tangent_cost\" for=\"edge\" "
                 "attr.name=\"tangent_cost\" attr.type=\"long\" />\n";
         file << "  <key id=\"adjoint_memory\" for=\"edge\" "
                 "attr.name=\"adjoint_memory\" attr.type=\"long\" />\n";
         file << "  <key id=\"has_model\" for=\"edge\" "
                 "attr.name=\"has_model\" attr.type=\"boolean\" />\n";
         file << "  <graph id=\"G\" edgedefault=\"directed\" "
                 "parse.nodeids=\"free\" parse.edgeids=\"canonical\" "
                 "parse.order=\"nodesfirst\">\n";
         file << std::format(
              "    <data key=\"fma_upper_bound\">{}</data>\n", optimized_cost);

         jacobians.at(0).print_graphml_input_node(file);
         for (std::size_t i = 0; i < jacobians.size(); ++i) {
            jacobians.at(i).print_graphml_output_node(file);
         }
         for (std::size_t i = 0; i < jacobians.size(); ++i) {
            jacobians.at(i).print_graphml_edge(file);
         }

         file << "  </graph>\n";
         file << "</graphml>\n";
         file.close();
      } else {
         throw std::runtime_error("Unable to open file");
      }
   }
};

}  // end namespace jcdp

// >>>>>>>>>>>>>>>> INCLUDE TEMPLATE AND INLINE DEFINITIONS <<<<<<<<<<<<<<<<< //

// #include "util/impl/jacobian.inl"  // IWYU pragma: export

#endif  // JCDP_JACOBIAN_CHAIN_HPP_
