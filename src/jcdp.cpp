#include <filesystem>
#include <iostream>

#include "jcdp/generator.hpp"
#include "jcdp/graphml.hpp"
#include "jcdp/jacobian_chain.hpp"
#include "jcdp/operation.hpp"
#include "jcdp/optimizer/dynamic_programming.hpp"
#include "jcdp/sequence.hpp"

int main(int argc, char* argv[]) {

   jcdp::JacobianChainGenerator jcgen;
   jcdp::optimizer::DynamicProgrammingOptimizer solver;

   if (argc < 2) {
      jcgen.print_help(std::cout);
      solver.print_help(std::cout);
      return -1;
   }

   const std::filesystem::path config_filename(argv[1]);
   try {
      solver.parse_config(config_filename, true);
      jcgen.parse_config(config_filename, true);
      jcgen.init_rng();
   } catch (const std::runtime_error& bcfe) {
      std::cerr << bcfe.what() << std::endl;
      return -1;
   }

   std::filesystem::path output_dir = ".";
   if (argc > 2) {
      output_dir = std::filesystem::path(argv[2]);
   }

   std::println("Chain generator properties:");
   jcgen.print_values(std::cout);

   std::println("\nSolver properties:");
   solver.print_values(std::cout);

   jcdp::JacobianChain chain;
   if (jcgen.next(chain)) {
      solver.init(chain);

      auto start = std::chrono::high_resolution_clock::now();
      solver.solve();
      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> duration = end - start;

      std::println("\nSolve duration: {} seconds", duration.count());
      std::println(
           "Tangent cost: {}",
           chain.get_jacobian(chain.length() - 1, 0).fma<jcdp::Mode::TANGENT>());
      std::println(
           "Adjoint cost: {}",
           chain.get_jacobian(chain.length() - 1, 0).fma<jcdp::Mode::ADJOINT>());
      std::println("Optimized cost: {}\n", chain.optimized_costs.back());

      jcdp::Sequence seq = solver.get_sequence();
      std::println("{}", seq);
      jcdp::write_graphml(output_dir, chain);
   }

   return 0;
}
