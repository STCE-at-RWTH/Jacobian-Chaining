#ifndef JCDP_GRAHPML_HPP_
#define JCDP_GRAHPML_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <cstddef>
#include <format>
#include <fstream>
#include <filesystem>

#include "jcdp/jacobian.hpp"
#include "jcdp/jacobian_chain.hpp"
#include "jcdp/operation.hpp"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

namespace jcdp {
namespace graphml {

inline auto write_header(std::ofstream& file) -> void {
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
}

inline auto write_footer(std::ofstream& file) -> void {
   file << "  </graph>\n";
   file << "</graphml>\n";
}

inline auto write_node(
     std::ofstream& file, const std::size_t index, const std::size_t size)
     -> void {
   file << std::format("    <node id=\"{}\">\n", index);
   file << std::format("      <data key=\"index\">{}</data>\n", index);
   file << std::format("      <data key=\"size\">{}</data>\n", size);
   file << "    </node>\n";
}

inline auto write_input_node(std::ofstream& file, const Jacobian& jac) -> void {
   write_node(file, jac.i, jac.n);
}

inline auto write_output_node(std::ofstream& file, const Jacobian& jac)
     -> void {
   write_node(file, jac.j, jac.m);
}

inline auto write_edge(std::ofstream& file, const Jacobian& jac) -> void {
   file << std::format(
        "    <edge id=\"{}\" source=\"{}\" target=\"{}\">\n", jac.i, jac.i,
        jac.j);
   file << std::format(
        "      <data key=\"adjoint_cost\">{}</data>\n",
        jac.template single_evaluation_fma<Mode::ADJOINT>());
   file << std::format(
        "      <data key=\"tangent_cost\">{}</data>\n",
        jac.template single_evaluation_fma<Mode::TANGENT>());
   file << std::format(
        "      <data key=\"adjoint_memory\">{}</data>\n", jac.edges_in_dag);
   file << "      <data key=\"has_model\">1</data>\n";
   file << "    </edge>\n";
}

}  // end namespace graphml

inline auto write_graphml(
     const std::filesystem::path& output_dir, const JacobianChain& chain,
     const std::optional<std::size_t> threads = {}) -> void {

   const std::size_t t = threads.value_or(chain.optimized_costs.size() - 1);
   std::ofstream file(output_dir / std::format("chain_{}_{}_{}.xml", chain.jacobians.size(), t, chain.id));
   if (file.is_open()) {
      graphml::write_header(file);
      file << std::format(
           "    <data key=\"fma_upper_bound\">{}</data>\n", chain.optimized_costs[t]);

      graphml::write_input_node(file, chain.jacobians.at(0));
      for (std::size_t i = 0; i < chain.jacobians.size(); ++i) {
         graphml::write_output_node(file, chain.jacobians.at(i));
      }
      for (std::size_t i = 0; i < chain.jacobians.size(); ++i) {
         graphml::write_edge(file, chain.jacobians.at(i));
      }

      graphml::write_footer(file);
      file.close();
   } else {
      throw std::runtime_error("Unable to open file");
   }
}

inline auto write_graphml_for_all_solutions(
     const std::filesystem::path& output_dir, const JacobianChain& chain) -> void {

   if (chain.optimized_costs.size() > 1) {
      for (std::size_t threads = 1; threads < chain.optimized_costs.size(); ++threads) {
         write_graphml(output_dir, chain, threads);
      }
   } else {
      write_graphml(output_dir, chain);
   }
}

}  // end namespace jcdp

// >>>>>>>>>>>>>>>> INCLUDE TEMPLATE AND INLINE DEFINITIONS <<<<<<<<<<<<<<<<< //

// #include "jcdp/jacobian.inl"  // IWYU pragma: export

#endif  // JCDP_JACOBIAN_HPP_
