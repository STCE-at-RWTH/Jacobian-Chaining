#ifndef JCDP_OPERATION_HPP_
#define JCDP_OPERATION_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <cassert>
#include <string>

namespace jcdp {

enum class Operation { NONE = 0, MULTIPLICATION, ACCUMULATION, ELIMINATION };

enum class Mode { NONE = 0, TANGENT, ADJOINT };

inline auto to_string(const Mode mode) -> std::string {
   assert(mode != Mode::NONE);

   if (mode == Mode::ADJOINT) {
      return "ADJ";
   } else {
      return "TAN";
   }
}

}  // end namespace jcdp

#endif  // JCDP_OPERATION_HPP_
