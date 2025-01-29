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

inline auto write_header(std::ofstream& file, const JacobianChain& chain) -> void {
   file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
   file << "<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\" "
           "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
           "xsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns "
           "http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd\">\n";
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

   constexpr std::string_view key_str {
        "  <key id=\"fma_upper_bound_{}\" for=\"graph\" attr.name=\"fma_upper_bound_{}\" attr.type=\"long\" />\n"};
   if (chain.optimized_costs.size() > 1) {
      for (std::size_t threads = 1; threads < chain.optimized_costs.size(); ++threads) {
         file << std::format(key_str, threads, threads);
      }
   } else {
      file << std::format(key_str, chain.length(), chain.length());
   }

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

inline auto write_optimized_costs(
     std::ofstream& file, const JacobianChain& chain) -> void {

   constexpr std::string_view data_str {"    <data key=\"fma_upper_bound_{}\">{}</data>\n"};
   if (chain.optimized_costs.size() > 1) {
      for (std::size_t threads = 1; threads < chain.optimized_costs.size(); ++threads) {
         file << std::format(data_str, threads, chain.optimized_costs[threads]);
      }
   } else {
      file << std::format(data_str, chain.length(), chain.optimized_costs[0]);
   }
}

}  // end namespace graphml

inline auto write_graphml(
     const std::filesystem::path& output_dir, const JacobianChain& chain) -> void {

   std::filesystem::create_directories(output_dir);
   std::filesystem::path filename = output_dir;
   filename /= std::format("chain_{}_{}.xml", chain.jacobians.size(), chain.id);

   std::ofstream file(filename);
   if (file.is_open()) {
      graphml::write_header(file, chain);
      graphml::write_optimized_costs(file, chain);
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

}  // end namespace jcdp

// >>>>>>>>>>>>>>>> INCLUDE TEMPLATE AND INLINE DEFINITIONS <<<<<<<<<<<<<<<<< //

// #include "jcdp/jacobian.inl"  // IWYU pragma: export

#endif  // JCDP_JACOBIAN_HPP_
