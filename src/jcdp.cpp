#include <iostream>
#include <filesystem>

#include "jcdp/dp_solver.hpp"
#include "jcdp/generator.hpp"
#include "jcdp/jacobian_chain.hpp"
#include "jcdp/operation.hpp"
#include "jcdp/graphml.hpp"

int main(int argc, char* argv[]) {

   jcdp::JacobianChainGenerator jcgen;
   jcdp::DPSolver solver;

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

   jcdp::JacobianChain chain;
   if (jcgen.next(chain)) {
      solver.init(chain);

      auto start = std::chrono::high_resolution_clock::now();
      solver.solve();
      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> duration = end - start;

      std::println("Solve duration: {} seconds", duration.count());
      std::println(
         "Tangent cost: {}",
         chain.subchain_fma<jcdp::Mode::TANGENT>(chain.length() - 1, 0, 0));
      std::println(
         "Adjoint cost: {}",
         chain.subchain_fma<jcdp::Mode::ADJOINT>(
               chain.length() - 1, 0, chain.length() - 1));

      if (chain.optimized_costs.size() > 1) {
         for (std::size_t threads = 1; threads < chain.optimized_costs.size(); ++threads) {
            std::println("Optimized cost ({}): {}", threads, chain.optimized_costs[threads]);
         }
      } else {
         std::println("Optimized cost (unlimited): {}", chain.optimized_costs[0]);
      }

      std::println();
      solver.print_sequence();
      jcdp::write_graphml(output_dir, chain);
   }

   return 0;
}
