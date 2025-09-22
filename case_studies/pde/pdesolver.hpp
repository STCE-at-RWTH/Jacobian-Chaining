#pragma once

#include <cmath>
#include <vector>

#include "ad_data_view.hpp"
#include "lapack.hpp"

namespace pde {

template<typename ATYPE>
class CrankNicholson {
 public:
   const ACTIVE_INPUTS<ATYPE>& X;
   const PASSIVE_INPUTS& XP;
   std::vector<ATYPE> LHSj_dl, LHSj_d, LHSj_du, Vprev, Vcurr;
   ATYPE* RHSj;
   ATYPE xmin, xmax, tmax;
   double tmin;

   CrankNicholson(const ACTIVE_INPUTS<ATYPE>& X, const PASSIVE_INPUTS& XP)
      : X(X), XP(XP), LHSj_dl(XP.N - 1), LHSj_d(XP.N), LHSj_du(XP.N - 1),
        Vprev(XP.N + 2), Vcurr(XP.N + 2) {}

   void swapVs() {
      std::swap(Vcurr, Vprev);
   }

 private:
   CrankNicholson();
   CrankNicholson(const CrankNicholson&);
   CrankNicholson operator=(const CrankNicholson&);

 public:
   void setBoundaryConditions(std::size_t ts, std::vector<ATYPE>& V);
   void prepareLHS(std::size_t ts);
   void prepareRHS(std::size_t);
   void solveTridiagonalSystem();
};

template<typename ATYPE>
void CrankNicholson<ATYPE>::setBoundaryConditions(
     std::size_t ts, std::vector<ATYPE>& V) {
   const ATYPE dx = (xmax - xmin) / static_cast<double>(XP.N + 1);
   const ATYPE dt = X.T / static_cast<double>(XP.M + 1);

   if (ts == XP.M + 1) {
      for (std::size_t i = 1; i < XP.N + 1; i++) {
         const ATYPE xi = xmin + static_cast<double>(i) * dx;
         const ATYPE Si = exp(xi);
         if (Si - X.K > 0.0)
            V[i] = Si - X.K;
         else
            V[i] = 0.0;
      }
   }
   V[0] = 0.0;
   const ATYPE Smax = exp(xmax);
   const ATYPE t_j = static_cast<double>(ts) * dt;
   V[XP.N + 1] = Smax - exp(-X.r * (X.T - t_j)) * X.K;
}

template<typename ATYPE>
void CrankNicholson<ATYPE>::prepareLHS(const std::size_t j_time) {
   const std::size_t j = j_time;
   const ATYPE dt = (tmax - tmin) / static_cast<double>(XP.M + 1);
   const ATYPE t = tmin + static_cast<double>(j) * dt;
   const ATYPE dx = (xmax - xmin) / static_cast<double>(XP.N + 1);
   const ATYPE alpha = dt / (dx * dx);
   const ATYPE z = 1.0 + X.r * dt;

   std::size_t row = 0;
   ATYPE x = xmin + static_cast<double>(row + 1) * dx;
   ATYPE u_ij, l_ij, c_ij;
   ATYPE vhat = X.sigmaSq(x, t);
   u_ij = 0.5 * alpha * (vhat + dx * (X.r - 0.5 * vhat));
   c_ij = -alpha * vhat;
   LHSj_du[row] = -0.5 * u_ij;
   LHSj_d[row] = z - 0.5 * c_ij;
   for (row = 1; row < XP.N - 1; row++) {
      x = xmin + static_cast<double>(row + 1) * dx;
      vhat = X.sigmaSq(x, t);
      u_ij = 0.5 * alpha * (vhat + dx * (X.r - 0.5 * vhat));
      l_ij = 0.5 * alpha * (vhat - dx * (X.r - 0.5 * vhat));
      c_ij = -alpha * vhat;
      LHSj_dl[row - 1] = -0.5 * l_ij;
      LHSj_d[row] = z - 0.5 * c_ij;
      LHSj_du[row] = -0.5 * u_ij;
   }
   row = XP.N - 1;
   x = xmin + static_cast<double>(row + 1) * dx;
   vhat = X.sigmaSq(x, t);
   l_ij = 0.5 * alpha * (vhat - dx * (X.r - 0.5 * vhat));
   c_ij = -alpha * vhat;
   LHSj_dl[row - 1] = -0.5 * l_ij;
   LHSj_d[row] = z - 0.5 * c_ij;
}

template<typename ATYPE>
void CrankNicholson<ATYPE>::prepareRHS(const std::size_t j_time) {
   const std::size_t j = j_time;
   const ATYPE dt = (tmax - tmin) / static_cast<double>(XP.M + 1);
   const ATYPE t = tmin + static_cast<double>(j) * dt;
   const ATYPE dx = (xmax - xmin) / static_cast<double>(XP.N + 1);
   const ATYPE alpha = dt / (dx * dx);
   std::vector<ATYPE> RHSj_d(XP.N), RHSj_du(XP.N - 1), RHSj_dl(XP.N - 1);

   std::size_t row = 0;
   ATYPE c_ij, u_ij, l_ij;
   ATYPE x = xmin + static_cast<double>(row + 1) * dx;
   ATYPE vhat = X.sigmaSq(x, t);
   u_ij = 0.5 * alpha * (vhat + dx * (X.r - 0.5 * vhat));
   c_ij = -alpha * vhat;
   RHSj_du[row] = 0.5 * u_ij;
   RHSj_d[row] = 0.5 * c_ij + 1.0;
   for (row = 1; row < XP.N - 1; row++) {
      x = xmin + static_cast<double>(row + 1) * dx;
      vhat = X.sigmaSq(x, t);
      u_ij = 0.5 * alpha * (vhat + dx * (X.r - 0.5 * vhat));
      l_ij = 0.5 * alpha * (vhat - dx * (X.r - 0.5 * vhat));
      c_ij = -alpha * vhat;
      RHSj_dl[row - 1] = 0.5 * l_ij;
      RHSj_d[row] = 0.5 * c_ij + 1.0;
      RHSj_du[row] = 0.5 * u_ij;
   }
   row = XP.N - 1;
   x = xmin + static_cast<double>(row + 1) * dx;
   vhat = X.sigmaSq(x, t);
   l_ij = 0.5 * alpha * (vhat - dx * (X.r - 0.5 * vhat));
   c_ij = -alpha * vhat;
   RHSj_dl[row - 1] = 0.5 * l_ij;
   RHSj_d[row] = 0.5 * c_ij + 1.0;

   row = 0;
   RHSj[row] = RHSj_d[row] * Vprev[1] + RHSj_du[row] * Vprev[2];
   x = xmin + static_cast<double>(row + 1) * dx;
   vhat = X.sigmaSq(x, t);
   l_ij = 0.5 * alpha * (vhat - dx * (X.r - 0.5 * vhat));
   RHSj[row] = RHSj[row] + l_ij * 0.5 * (Vcurr[0] + Vprev[0]);
   for (row = 1; row < XP.N - 1; row++)
      RHSj[row] = RHSj_dl[row - 1] * Vprev[row] + RHSj_d[row] * Vprev[row + 1] +
                  RHSj_du[row] * Vprev[row + 2];

   row = XP.N - 1;
   x = xmin + static_cast<double>(row + 1) * dx;
   RHSj[row] = RHSj_dl[row - 1] * Vprev[row] + RHSj_d[row] * Vprev[row + 1];
   vhat = X.sigmaSq(x, t);
   u_ij = 0.5 * alpha * (vhat + dx * (X.r - 0.5 * vhat));
   RHSj[row] = RHSj[row] + u_ij * 0.5 * (Vcurr[XP.N + 1] + Vprev[XP.N + 1]);
}

template<typename ATYPE>
void CrankNicholson<ATYPE>::solveTridiagonalSystem() {
   std::vector<int> LHSj_ipiv(XP.N);
   std::vector<ATYPE> LHSj_du2(XP.N - 2);

   int info = 0;
   dgttrf(
        static_cast<int>(XP.N), LHSj_dl.data(), LHSj_d.data(), LHSj_du.data(),
        LHSj_du2.data(), LHSj_ipiv.data(), &info);
   dgttrs(
        "N", static_cast<int>(XP.N), 1, LHSj_dl.data(), LHSj_d.data(),
        LHSj_du.data(), LHSj_du2.data(), LHSj_ipiv.data(), RHSj,
        static_cast<int>(XP.N), &info);
}

template<typename ATYPE>
void price(
     const ACTIVE_INPUTS<ATYPE>& X, const PASSIVE_INPUTS& XP,
     ACTIVE_OUTPUTS<ATYPE>& Y) {
   CrankNicholson<ATYPE> CN(X, XP);
   const double tmin = 0.0;
   CN.tmin = tmin;
   const ATYPE tmax = X.T;
   CN.tmax = tmax;
   const ATYPE logS0 = log(X.S0);
   const ATYPE atmvol = sqrt(X.T * X.sigmaSq(logS0, X.T));

   const double C = 10;
   const ATYPE dx = 2.0 * C * atmvol / static_cast<double>(XP.N + 1);
   const ATYPE xmin = logS0 - static_cast<double>(XP.N) / 2 * dx;
   CN.xmin = xmin;
   const ATYPE xmax = xmin + static_cast<double>(XP.N + 1) * dx;
   CN.xmax = xmax;

   CN.setBoundaryConditions(XP.M + 1, CN.Vprev);
   for (std::size_t j = 0; j <= XP.M; j++) {
      CN.setBoundaryConditions(XP.M - j, CN.Vcurr);
      CN.prepareLHS(XP.M - j);
      CN.RHSj = &CN.Vcurr[1];
      CN.prepareRHS(XP.M - j);
      CN.solveTridiagonalSystem();
      CN.swapVs();
   }
   Y.V = CN.Vprev[XP.N / 2];
}

}  // namespace pde
