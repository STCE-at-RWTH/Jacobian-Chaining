#pragma once

#include "ad/ad.hpp"
#include "local_vols.hpp"

//** required to be compatible with C -- only use two-digit exponent
//** in output of scientific numbers.
#if defined(WIN32) && _MSC_VER < 1900
struct outputformatter {
   outputformatter() {
      _set_output_format(_TWO_DIGIT_EXPONENT);
   }
};

static outputformatter of;
#endif

// collection of independent variables
template<typename ATYPE>
struct ACTIVE_INPUTS {
   ATYPE S0, r, K, T;
   LocalVolSurface<ATYPE> sigmaSq;

   ACTIVE_INPUTS(
        const double& S0, const double& r, const double& K, const double& T,
        std::istream& in)
      : S0(S0), r(r), K(K), T(T), sigmaSq(in) {}

   ACTIVE_INPUTS(
        const ATYPE& S0, const ATYPE& r, const ATYPE& K, const ATYPE& T,
        const LocalVolSurface<ATYPE>& sigmaSq)
      : S0(S0), r(r), K(K), T(T), sigmaSq(sigmaSq) {}

   ~ACTIVE_INPUTS() {}

   using AD_VALUE_TYPE = double;

   ACTIVE_INPUTS<AD_VALUE_TYPE> get_value() const {
      ACTIVE_INPUTS<AD_VALUE_TYPE> R(
           S0.m_value, r.m_value, K.m_value, T.m_value, sigmaSq.get_value());
      return R;
   }
};

// collection of passive parameters
struct PASSIVE_INPUTS {
   std::size_t N;
   std::size_t M;
   mutable double rngseed[6];

   PASSIVE_INPUTS(const std::size_t& N, const std::size_t& M) : N(N), M(M) {
      for (std::size_t i = 0; i < 6; i++)
         rngseed[i] = static_cast<double>(i + 1);
   }

   ~PASSIVE_INPUTS() {}

   void reseed() {
      for (std::size_t i = 0; i < 6; i++)
         rngseed[i] = static_cast<double>(i + 1);
   }
};

// collection of dependent variables
template<typename ATYPE>
struct ACTIVE_OUTPUTS {
   ATYPE V;
};

// collection of dependent variables
struct PASSIVE_OUTPUTS {
   double ci;
};
