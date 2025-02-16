/******************************************************************************
 * @file jcdp/util/dot_writer.hpp
 *
 * @brief This file is part of the JCDP package. It provides a several helper
 *        functions that convert elimuination sequences in to dot graphs.
 ******************************************************************************/

#ifndef JCDP_UTIL_DOT_WRITER_HPP_
#define JCDP_UTIL_DOT_WRITER_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <string>

#include "jcdp/sequence.hpp"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

namespace jcdp::util {

inline auto write_dot(const Sequence& sequence, const std::string& name)
     -> void {

   std::filesystem::path output_file = "sequence_" + name + ".dot";
   std::ofstream out(output_file);
   if (!out) {
      std::println(std::cerr, "Failed to open {}", output_file.string());
      return;
   }

   std::println(out, "digraph G {{");
   for (std::size_t i = 0; i < sequence.length(); ++i) {
      std::println(out, "  {} [label=\"{}\"]", i, sequence[i]);
   }

   for (std::size_t i = 0; i < sequence.length(); ++i) {
      for (std::size_t j = 0; j < sequence.length(); ++j) {
         if (sequence[i] > sequence[j]) {
            std::println(out, "  {} -> {}", i, j);
         }
      }
   }

   std::println(out, "}}");

   out.close();
}

}  // end namespace jcdp::util

// >>>>>>>>>>>>>>>> INCLUDE TEMPLATE AND INLINE DEFINITIONS <<<<<<<<<<<<<<<<< //

#endif  // JCDP_UTIL_DOT_WRITER_HPP_
