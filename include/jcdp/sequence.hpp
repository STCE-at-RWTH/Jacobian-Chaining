#ifndef JCDP_SEQUENCE_HPP_
#define JCDP_SEQUENCE_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <algorithm>
#include <limits>
#include <numeric>
#include <optional>
#include <deque>
#include <vector>

#include "jcdp/operation.hpp"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

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
            assert(op.is_scheduled);
            cost = std::max(cost, op.start_time + op.fma);
         }
      }

      return cost;
   }

   inline auto sequential_makespan() -> std::size_t {
      return std::reduce(
           cbegin(), cend(), static_cast<std::size_t>(0),
           [](const std::size_t cost, const Operation& op) -> std::size_t {
              return cost + op.fma;
           });
   }

   inline auto children(const std::size_t op_idx) const
        -> std::vector<std::size_t> {
      assert(op_idx < length());

      std::vector<std::size_t> child_ops;
      for (std::size_t i = 0; i < length(); ++i) {
         if (at(i) > at(op_idx)) {
            child_ops.push_back(i);
         }
      }

      return child_ops;
   }

   inline auto parent(const std::size_t op_idx) const
        -> std::optional<std::size_t> {
      assert(op_idx < length());

      std::vector<std::size_t> child_ops;
      for (std::size_t i = 0; i < length(); ++i) {
         if (at(i) < at(op_idx)) {
            return i;
         }
      }

      return {};
   }

   inline auto level(const std::size_t op_idx) const -> std::size_t {
      std::optional<std::size_t> p = parent(op_idx);
      if (p.has_value()) {
         return level(p.value()) + 1;
      }
      return 1;
   }

   inline auto critical_path() const -> std::size_t {

      std::size_t makespan = 0;
      for (std::size_t op_idx = 0; op_idx < length(); ++op_idx) {
         makespan = std::max(makespan, critical_path(op_idx));
      }
      return makespan;
   }

   inline auto critical_path(const std::size_t op_idx, std::size_t start_time = 0) const -> std::size_t {

      start_time = std::max(start_time, at(op_idx).start_time);
      const std::size_t end_time = start_time + at(op_idx).fma;
      std::optional<std::size_t> p = parent(op_idx);
      if (p.has_value()) {
         return critical_path(p.value(), end_time);
      }
      return end_time;
   }

   inline auto is_schedulable(const std::size_t op_idx) const -> bool {

      return std::all_of(
           cbegin(), cend(),
           [this, op_idx](const Operation& op) -> bool {
              if (at(op_idx) < op) {
                 return op.is_scheduled;
              }

              return true;
           });
   }

   inline auto is_scheduled() const -> bool {

      return std::all_of(
           cbegin(), cend(),
           [](const Operation& op) -> bool {
              return op.is_scheduled;
           });
   }

   inline auto earliest_start(const std::size_t op_idx) const
        -> std::size_t {

      return std::reduce(
           cbegin(), cend(), static_cast<std::size_t>(0),
           [this, op_idx](
                const std::size_t start, const Operation& op) -> std::size_t {
              if (at(op_idx) < op) {
                 return std::max(start, op.start_time + op.fma);
              }

              return start;
           });
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

   inline auto length() const -> std::size_t {
      return size();
   }

   inline static auto make_max() -> Sequence {
      return Sequence(Operation {
           .fma = std::numeric_limits<std::size_t>::max(),
           .is_scheduled = true});
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

#endif  // JCDP_SEQUENCE_HPP_
