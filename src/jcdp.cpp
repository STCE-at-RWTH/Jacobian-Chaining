
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "jcdp/jacobian_chain.hpp"


int main(int argc, char* argv[]) {
   jcdp::JacobianChainProperties p;

   if (argc != 2) {
      p.print_help(std::cout);
      return -1;
   }

   try {
      p.parse_config(std::ifstream(argv[1]));
   } catch (const std::runtime_error& bcfe) {
      std::cout << bcfe.what() << std::endl;
      return -1;
   }

   // p.print_values(std::cout);

   const jcdp::JacobianChain chain = jcdp::JacobianChain::generate_random(p);
   chain.write_graphml("out.xml");

   return 0;
}