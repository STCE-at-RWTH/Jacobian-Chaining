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

class Sequence {

 public:
   Sequence() = default;

   Sequence(Operation&& rhs) : m_operations {rhs} {};

   inline auto makespan(const std::optional<std::size_t> thread = {})
        -> std::size_t {
      std::size_t cost = 0;
      for (const Operation& op : m_operations) {
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
      m_operations.append_range(rhs.m_operations);
      return *this;
   }

   inline auto operator+(const Operation& rhs) -> const Sequence {
      Sequence res = *this;
      res += rhs;
      return res;
   }

   inline auto operator+=(const Operation& rhs) -> Sequence {
      m_operations.push_back(rhs);
      return *this;
   }

   inline auto operator[](const std::size_t idx) -> Operation& {
      return m_operations[idx];
   }

   inline auto operator[](const std::size_t idx) const -> const Operation& {
      return m_operations[idx];
   }

   inline auto length() -> std::size_t {
      return m_operations.size();
   }

   inline auto begin() -> std::deque<Operation>::iterator {
      return m_operations.begin();
   }

   inline auto begin() const -> std::deque<Operation>::const_iterator {
      return m_operations.begin();
   }

   inline auto cbegin() const -> std::deque<Operation>::const_iterator {
      return m_operations.cbegin();
   }

   inline auto end() -> std::deque<Operation>::iterator {
      return m_operations.end();
   }

   inline auto end() const -> std::deque<Operation>::const_iterator {
      return m_operations.end();
   }

   inline auto cend() const -> std::deque<Operation>::const_iterator {
      return m_operations.cend();
   }

   inline auto back() -> Operation& {
      return m_operations.back();
   }

   inline auto back() const -> const Operation& {
      return m_operations.back();
   }

   inline auto front() -> Operation& {
      return m_operations.front();
   }

   inline auto front() const -> const Operation& {
      return m_operations.front();
   }

   inline static auto make_max() -> Sequence {
      return Sequence(
           Operation {.fma = std::numeric_limits<std::size_t>::max()});
   }

 private:
   std::deque<Operation> m_operations;
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
