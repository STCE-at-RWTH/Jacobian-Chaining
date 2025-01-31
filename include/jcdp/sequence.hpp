#ifndef JCDPm_operations_HPP_
#define JCDPm_operations_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <algorithm>
#include <cassert>
#include <limits>
#include <sstream>
#include <string>

#include "jcdp/operation.hpp"

namespace jcdp {

class Sequence : public std::deque<Operation> {

 public:
   Sequence() = default;

   Sequence(Operation&& rhs) : std::deque<Operation> {rhs} {};

   inline auto makespan(const std::optional<std::size_t> thread = {})
        -> std::size_t {
      std::size_t cost = 0;
      for (const Operation& op : *this) {
         if (op.thread == thread.value_or(op.thread)) {
            cost = std::max(cost, op.start_time + op.fma);
         }
      }

      return cost;
   }

   inline auto operator+(const Sequence& rhs) -> const Sequence {
      Sequence res = *this;
      res += rhs;
      return res;
   }

   inline auto operator+=(const Sequence& rhs) -> Sequence {
      append_range(rhs);
      return *this;
   }

   inline auto operator+(const Operation& rhs) -> const Sequence {
      Sequence res = *this;
      res += rhs;
      return res;
   }

   inline auto operator+=(const Operation& rhs) -> Sequence {
      push_back(rhs);
      return *this;
   }


   inline auto length() -> std::size_t {
      return size();
   }


   inline static auto make_max() -> Sequence {
      return Sequence(
           Operation {.fma = std::numeric_limits<std::size_t>::max()});
   }
};

}  // end namespace jcdp

template<>
struct std::formatter<jcdp::Sequence> {

   template<class ParseContext>
   constexpr auto parse(ParseContext& ctx) -> ParseContext::iterator {
      return ctx.begin();
   }

   template<class FmtContext>
   auto format(const jcdp::Sequence& seq, FmtContext& ctx) const
        -> FmtContext::iterator {

      typename FmtContext::iterator out = ctx.out();
      for (const jcdp::Operation& op : seq) {
         out = std::format_to(out, "{}\n", op);
      }

      return out;
   }
};

#endif  // JCDPm_operations_HPP_
