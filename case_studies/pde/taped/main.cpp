#include <cassert>
#include <fstream>
#include <iostream>

#include "ad/ad.hpp"

using AD_TYPE = ad::noet::active_t<double>;

#include "../ad_data_view.hpp"
#include "../config.hpp"
#include "../pdesolver.hpp"

int main(int argc, char* argv[]) {
   if (argc != 2) {
      std::cerr << "Please specify input scenario file, e.g. scenario_1.in\n";
      return EXIT_FAILURE;
   }
   std::cout.precision(5);
   std::ifstream vols(argv[1]);
   if (vols.fail()) {
      std::cerr << "Cannot open scenario file '" << argv[1] << "'\n";
      return EXIT_FAILURE;
   }
   ACTIVE_INPUTS<AD_TYPE> X(S0, r, K, T, vols);
   vols.close();
   PASSIVE_INPUTS XP(N, M);
   ACTIVE_OUTPUTS<AD_TYPE> Y;

   // XM[xmsz] is an array pointing to all active inputs
   std::size_t xmsz = 4 + X.sigmaSq.a.size() + X.sigmaSq.b.size();
   std::vector<AD_TYPE*> XM(xmsz);
   XM[0] = &X.S0;
   XM[1] = &X.r;
   XM[2] = &X.K;
   XM[3] = &X.T;
   for (std::size_t i = 0; i < X.sigmaSq.a.size(); i++)
      XM[i + 4] = &X.sigmaSq.a[i];
   for (std::size_t i = 0; i < X.sigmaSq.b.size(); i++)
      XM[i + 4 + X.sigmaSq.a.size()] = &X.sigmaSq.b[i];

   for (std::size_t i = 0; i < xmsz; i++) {
      XM[i]->register_input();
   }

   pde::price(X, XP, Y);

   // std::cerr << "Tape size: " << dco::size_of(tape) << "B\n\n";

   Y.V.register_output();

   tape->interpret_adjoint();

   std::cout << "Y=" << Y.V.value() << "\n";

   for (std::size_t i = 0; i < xmsz; i++)
      std::cout << "dY/dX[" << i << "]=" << dco::derivative(*XM[i]) << "\n";
}
