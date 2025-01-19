#include <fstream>
#include <iostream>
#include <stdexcept>

#include "jcdp/dp_solver.hpp"
#include "jcdp/jacobian_chain.hpp"

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

   // p.print_values(std::cout);

   jcdp::JacobianChain chain = jcdp::JacobianChain::generate_random(jcp);
   chain.write_graphml("out.xml");

   solver.init(std::move(chain));
   solver.solve();
   solver.print_sequence();

   return 0;
}
