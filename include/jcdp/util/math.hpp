#ifndef JCDP_UTIL_MATH_HPP_
#define JCDP_UTIL_MATH_HPP_

#include <cstddef>

namespace jcdp::util {

inline auto nCr(std::size_t n, std::size_t r) -> std::size_t {
   if (r > n) {
      return 0;
   }
   if (r == 0 || r == n) {
      return 1;
   }

   if (r > n / 2) {
      r = n - r;
   }

   std::size_t res = 1;
   for (std::size_t i = 1; i <= r; ++i) {
      res = res * (n - i + 1) / i;
   }
   return res;
}

}  // namespace jcdp::util

#endif  // JCDP_UTIL_MATH_HPP_
