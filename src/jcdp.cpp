#include <fstream>
#include <iostream>
#include <stdexcept>

#include "jcdp/dp_solver.hpp"
#include "jcdp/jacobian_chain.hpp"
#include "jcdp/operation.hpp"

int main(int argc, char* argv[]) {
   jcdp::JacobianChainProperties jcp;
   jcdp::DPSolver solver;

   if (argc != 2) {
      jcp.print_help(std::cout);
      solver.print_help(std::cout);
      return -1;
   }

   try {
      jcp.parse_config(std::ifstream(argv[1]), true);
      solver.parse_config(std::ifstream(argv[1]), true);
   } catch (const std::runtime_error& bcfe) {
      std::cout << bcfe.what() << std::endl;
      return -1;
   }

   jcdp::JacobianChain chain = jcdp::JacobianChain::generate_random(jcp);
   solver.init(chain);

   auto start = std::chrono::high_resolution_clock::now();
   std::size_t optimized_cost = solver.solve();
   auto end = std::chrono::high_resolution_clock::now();
   std::chrono::duration<double> duration = end - start;

   std::println("Solve duration: {} seconds", duration.count());
   std::println(
        "Tangent cost: {}",
        chain.subchain_fma<jcdp::Mode::TANGENT>(jcp.chain_length - 1, 0, 0));
   std::println(
        "Adjoint cost: {}",
        chain.subchain_fma<jcdp::Mode::ADJOINT>(
             jcp.chain_length - 1, 0, jcp.chain_length - 1));
   std::println("Optimized cost: {}\n", optimized_cost);

   solver.print_sequence();
   chain.write_graphml("chain.xml");

   return 0;
}
