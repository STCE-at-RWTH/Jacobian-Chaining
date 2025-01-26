#include <iostream>
#include <filesystem>

#include "jcdp/dp_solver.hpp"
#include "jcdp/generator.hpp"
#include "jcdp/jacobian_chain.hpp"
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

   std::filesystem::path output_dir;
   if (argc > 2) {
      output_dir = std::filesystem::path(argv[2]);
   }

   jcdp::JacobianChain chain;
   while (jcgen.next(chain)) {
      solver.init(chain);
      solver.solve();
      jcdp::write_graphml_for_all_solutions(output_dir, chain);
   }

   return 0;
}
