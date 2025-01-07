#ifndef JCDP_JACOBIAN_CHAIN_HPP_
#define JCDP_JACOBIAN_CHAIN_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <random>
#include <vector>

#include "jcdp/jacobian.hpp"
#include "jcdp/util/properties.hpp"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

namespace jcdp {

class JacobianChainProperties : public Properties {
 public:
   size_t chain_length = 1;
   std::pair<size_t, size_t> size_range = {1, 1};
   std::pair<size_t, size_t> cost_range = {1, 1};
   std::pair<double, double> memory_factor_range = {0.0, 2.0};
   std::pair<double, double> density_range = {0.0, 1.0};

   JacobianChainProperties() {
      register_property(
           chain_length, "length", "Length of the Jacobian Chain.");
      register_property(
           size_range, "size_range", "Range of the Jacobian dimensions.");
      register_property(
           cost_range, "cost_range",
           "Range of the tangent and adjoint evaluation costs.");
      register_property(
           memory_factor_range, "memory_range",
           "Range of the persistent memory factors (is multiplied by the "
           "adjoint fma).");
      register_property(
           density_range, "density_range",
           "Range of density percentages of the Jacobians. Used to calcluate "
           "number of non-zero entries and bandwidth.");
   }
};

struct JacobianChain {
   std::vector<Jacobian> jacobians;

   //! Generate a random Jacobian matrix.
   inline static auto generate_random(const JacobianChainProperties& p)
        -> JacobianChain {

      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<std::size_t> size_distribution(
           p.size_range.first, p.size_range.second);
      std::uniform_int_distribution<std::size_t> cost_distribution(
           p.cost_range.first, p.cost_range.second);
      std::uniform_real_distribution<double> mem_distribution(
           p.memory_factor_range.first, p.memory_factor_range.second);
      std::uniform_real_distribution<double> density_distribution(
           p.density_range.first, p.density_range.second);

      JacobianChain chain;
      chain.jacobians.reserve(p.chain_length);
      chain.jacobians.push_back(Jacobian::generate_random(
           gen, size_distribution, cost_distribution, mem_distribution,
           density_distribution));
      chain.jacobians[0].i = 0;
      chain.jacobians[0].j = 1;

      for (std::size_t i = 1; i < p.chain_length; ++i) {
         chain.jacobians.push_back(Jacobian::generate_random(
              gen, size_distribution, cost_distribution, mem_distribution,
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
         file << "  <key id=\"adjoint_cost\" for=\"edge\" "
                 "attr.name=\"adjoint_cost\" attr.type=\"long\" />\n";
         file << "  <key id=\"tangent_cost\" for=\"edge\" "
                 "attr.name=\"tangent_cost\" attr.type=\"long\" />\n";
         file << "  <key id=\"index\" for=\"node\" "
                 "attr.name=\"index\" attr.type=\"long\" />\n";
         file << "  <key id=\"size\" for=\"node\" "
                 "attr.name=\"size\" attr.type=\"long\" />\n";
         file << "  <graph id=\"G\" edgedefault=\"directed\" "
                 "parse.nodeids=\"free\" parse.edgeids=\"canonical\" "
                 "parse.order=\"nodesfirst\">\n";

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
