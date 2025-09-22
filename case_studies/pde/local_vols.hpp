#pragma once

#include <vector>

template<typename ATYPE>
struct LocalVolSurface {
   std::size_t m, n;
   std::vector<ATYPE> a, b;

   explicit LocalVolSurface() : m(0), n(0) {}

   LocalVolSurface(std::istream& in) {
      in >> m >> n;
      a.resize(m + 1);
      b.resize(n + 1);
      for (std::size_t i = 0; i <= m; i++) {
         in >> a[i];
      }
      for (std::size_t i = 0; i <= n; i++) {
         in >> b[i];
      }
   }

   // squared volatilities; usually through interpolation
   // from measured market data
   ATYPE operator()(const ATYPE& x, const ATYPE& t) const {
      // Horner
      ATYPE ax = a[m];
      for (std::size_t i = 1; i <= m; i++) {
         ax = a[m - i] + x * ax;
      }
      ATYPE bx = b[n];
      for (std::size_t i = 1; i <= n; i++) {
         bx = b[n - i] + x * bx;
      }
      return t * ax / bx;
   }
};
