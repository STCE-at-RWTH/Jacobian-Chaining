#ifndef JCDP_JACOBIAN_HPP_
#define JCDP_JACOBIAN_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <cstddef>
#include <format>
#include <fstream>

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

namespace jcdp {

struct Jacobian {
   //! Index of input variable.
   std::size_t i{0};
   //! Index of output variable.
   std::size_t j{0};

   //! Input size.
   std::size_t n{0};
   //! Output size.
   std::size_t m{0};

   //! Number of super-diagonals (upper bandwidth).
   std::size_t ku{0};
   //! Number of sub-diagonals (lower bandwidth).
   std::size_t kl{0};
   //! Number of non-zero elements (general sparsity).
   std::size_t non_zero_elements{0};

   //! Cost of a single tangent evaluation (y^(1) = F' * x^(1)).
   std::size_t tangent_fma{0};
   //! Cost of a single adjoint evaluation (x_(1) = y_(1) * F').
   std::size_t adjoint_fma{0};
   //! Memory requirement for a single adjoint evaluation (~ size of tape).
   std::size_t adjoint_persistent_memory{0};

   //! Generate a random Jacobian matrix.
   template<class Generator, class IntDistribution, class RealDistribution>
   inline static auto generate_random(
        Generator& gen, IntDistribution& size_distribution,
        IntDistribution& cost_distribution, RealDistribution& mem_distribution,
        RealDistribution& density_distribution,
        const std::optional<std::size_t> n = {}) -> Jacobian;

   inline auto print_graphml_input_node(std::ofstream& file) const -> void {
      print_graphml_node(file, i, n);
   }

   inline auto print_graphml_output_node(std::ofstream& file) const -> void {
      print_graphml_node(file, j, m);
   }

   inline auto print_graphml_edge(std::ofstream& file) const -> void {
      file << std::format(
           "    <edge id=\"{}\" source=\"{}\" target=\"{}\">\n", i, i, j);
      file << std::format(
           "      <data key=\"adjoint_cost\">{}</data>\n", adjoint_fma);
      file << std::format(
           "      <data key=\"tangent_cost\">{}</data>\n", tangent_fma);
      file << "    </edge>\n";
   }

 private:
   inline auto print_graphml_node(
        std::ofstream& file, const std::size_t index,
        const std::size_t size) const -> void {
      file << std::format("    <node id=\"{}\">\n", index);
      file << std::format("      <data key=\"index\">{}</data>\n", index);
      file << std::format("      <data key=\"size\">{}</data>\n", size);
      file << "    </node>\n";
   }
};

}  // end namespace jcdp

// >>>>>>>>>>>>>>>> INCLUDE TEMPLATE AND INLINE DEFINITIONS <<<<<<<<<<<<<<<<< //

#include "jcdp/jacobian.inl"  // IWYU pragma: export

#endif  // JCDP_JACOBIAN_HPP_
