#include <filesystem>
#include <iostream>

#include "jcdp/generator.hpp"
#include "jcdp/graphml.hpp"
#include "jcdp/jacobian_chain.hpp"
#include "jcdp/operation.hpp"
#include "jcdp/optimizer/branch_and_bound.hpp"
#include "jcdp/optimizer/dynamic_programming.hpp"
#include "jcdp/sequence.hpp"

int main(int argc, char* argv[]) {

   jcdp::JacobianChainGenerator jcgen;
   jcdp::optimizer::DynamicProgrammingOptimizer dp_solver;
   jcdp::optimizer::BranchAndBoundOptimizer bnb_solver;

   if (argc < 2) {
      jcgen.print_help(std::cout);
      dp_solver.print_help(std::cout);
      return -1;
   }

   const std::filesystem::path config_filename(argv[1]);
   try {
      dp_solver.parse_config(config_filename, true);
      bnb_solver.parse_config(config_filename, true);
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

   std::println("\ndp_solver properties:");
   dp_solver.print_values(std::cout);

   jcdp::JacobianChain chain;
   if (jcgen.next(chain)) {
      chain.init_subchains();

      std::println(
           "Tangent cost: {}",
           chain.get_jacobian(chain.length() - 1, 0).fma<jcdp::Mode::TANGENT>());
      std::println(
           "Adjoint cost: {}",
           chain.get_jacobian(chain.length() - 1, 0).fma<jcdp::Mode::ADJOINT>());

      dp_solver.init(chain);
      auto start_dp = std::chrono::high_resolution_clock::now();
      jcdp::Sequence dp_seq = dp_solver.solve();
      auto end_dp = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> duration = end_dp - start_dp;
      std::println("\nDP solve duration: {} seconds", duration.count());
      std::println("Optimized cost (DP): {}\n", dp_seq.makespan());
      std::println("{}", dp_seq);

      bnb_solver.init(chain);
      // bnb_solver.set_upper_bound(dp_seq.makespan());
      auto start_bnb = std::chrono::high_resolution_clock::now();
      jcdp::Sequence bnb_seq = bnb_solver.solve();
      auto end_bnb = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> duration_bnb = end_bnb - start_bnb;
      std::println("\nBnB solve duration: {} seconds", duration_bnb.count());
      std::println("Optimized cost (BnB): {}\n", bnb_seq.makespan());
      std::println("{}", bnb_seq);

      jcdp::write_graphml(output_dir, chain);
   }

   return 0;
}
