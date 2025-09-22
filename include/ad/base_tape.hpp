// ************************************************************************** //
//           Advanced Algorithmic Differentiation SS23 - Tutorial 5           //
//                                                                            //
//                        Linear Algebra & Vector mode                        //
//                                                                            //
// Copyright (C) 2023 Software and Tools for Computational Engineering (STCE) //
//               RWTH Aachen University - www.stce.rwth-aachen.de             //
// ************************************************************************** //

#ifndef AD_BASE_TAPE_HPP_
#define AD_BASE_TAPE_HPP_

#include "ad/vector.hpp"

// Include C++ standard header
#include <cassert>
#include <cstddef>
#include <vector>
#include <iostream>

// Include eigen
#include <Eigen/Dense>

// **************************** First order tape **************************** //

namespace ad {

template<typename T>
struct tape_edge {
   std::size_t dependee_pos;
   T partial;
};

template<typename T>
struct tape_entry {
   std::vector<tape_edge<T>> edges;

   //! Default constructor.
   tape_entry() = default;

   inline void add_edge(const std::size_t dependee_pos, const T partial) {
      edges.push_back({dependee_pos, partial});
   }
};

/******************************************************************************
 * @brief Abstract first-order adjoint tape base class.
 ******************************************************************************/
template<typename T>
struct base_tape {

   //! Vector of variable dependencies.
   std::vector<tape_entry<T>> m_nodes;

   //! Indices of independent variables.
   std::vector<std::size_t> m_independent_variables;

   //! Indices of dependent variables.
   std::vector<std::size_t> m_dependent_variables;

   //! Calculate size of vector of adjoints.
   virtual std::size_t derivative_vector_size() = 0;
   //! Postion in derivative vector that corresponds to a given tape position.
   virtual std::size_t derivative_idx(std::size_t) const = 0;

   std::size_t create_tape_entry();
   void record_dependency(std::size_t, std::size_t, T);
   void mark_independent_variable(std::size_t);
   void mark_dependent_variable(std::size_t);

   //! Reverse interpret the tape using a given adjoint vector.
   template<size_t VEC_SIZE>
   void reverse_interpret_from_to(
        derivative_vector_t<T, VEC_SIZE>&, std::size_t, std::size_t) const;

   //! Reverse interpret the tape using a given adjoint vector.
   template<size_t VEC_SIZE>
   void reverse_interpret(derivative_vector_t<T, VEC_SIZE>&) const;

   //! Forward interpret the tape using a given tangent vector.
   template<size_t VEC_SIZE>
   void forward_interpret_from_to(
        derivative_vector_t<T, VEC_SIZE>&, std::size_t, std::size_t) const;

   //! Forward interpret the tape using a given tangent vector.
   template<size_t VEC_SIZE>
   void forward_interpret(derivative_vector_t<T, VEC_SIZE>&) const;

   //! Interpret the tape using a given derivative vector.
   template<size_t VEC_SIZE>
   void interpret_from_to(
        derivative_vector_t<T, VEC_SIZE>&, std::size_t, std::size_t) const;

   //! Reset the tape.
   virtual void reset();
   // //! Create dot graph representation of the graph.
   // void todot(const std::string&) const;
   // //! Print partial derivatives and dependencies.
   // void print() const;
   // //! Print the size of the tape as well as the adjoint vector.
   // virtual void print_size() const;

   //  private:
   //    //! Empty debug output. Prints line break.
   //    void print_debug(std::ostream&) const;
   //    //! Debug output.
   //    template<typename T, typename... Args>
   //    void print_debug(std::ostream&, T, Args...) const;
};

template<typename T>
std::size_t base_tape<T>::create_tape_entry() {
   m_nodes.emplace_back();
   return m_nodes.size() - 1;
}

template<typename T>
void base_tape<T>::record_dependency(
     const std::size_t dependeent_pos, const std::size_t dependee_pos,
     T partial) {
   m_nodes[dependeent_pos].add_edge(dependee_pos, partial);
}

template<typename T>
void base_tape<T>::mark_independent_variable(std::size_t idx) {
   m_independent_variables.push_back(idx);
}

template<typename T>
void base_tape<T>::mark_dependent_variable(std::size_t idx) {
   m_dependent_variables.push_back(idx);
}

/******************************************************************************
 * @brief Reverse interpret the tape using a given adjoint vector.
 * @param[inout] adjoints Seeded adjoint vector.
 ******************************************************************************/
template<typename T>
template<size_t VEC_SIZE>
void base_tape<T>::reverse_interpret_from_to(
     derivative_vector_t<T, VEC_SIZE>& adjoints, std::size_t from,
     std::size_t to) const {

   assert(from < m_nodes.size());
   assert(to >= 0);
   assert(from >= to);

   for (std::size_t dependent_pos = from + 1; dependent_pos-- > to;) {
      for (const auto& [dependee_pos, partial] : m_nodes[dependent_pos].edges) {
         std::cout << adjoints[derivative_idx(dependee_pos)] << " [" << dependee_pos
            << "] <-- " << partial << " * "
            << adjoints[derivative_idx(dependent_pos)] << " [" << dependent_pos << "]\n";

         adjoints[derivative_idx(dependee_pos)] +=
              partial * adjoints[derivative_idx(dependent_pos)];
      }
   }
}

/******************************************************************************
 * @brief Reverse interpret the tape using a given adjoint vector.
 * @param[inout] adjoints Seeded adjoint vector.
 ******************************************************************************/
template<typename T>
template<size_t VEC_SIZE>
void base_tape<T>::reverse_interpret(
     derivative_vector_t<T, VEC_SIZE>& adjoints) const {
   reverse_interpret_from_to<VEC_SIZE>(adjoints, m_nodes.size() - 1, 0);
}

/******************************************************************************
 * @brief Forward interpret the tape using a given tangent vector.
 * @param[inout] tangents Seeded tangent vector.
 ******************************************************************************/
template<typename T>
template<size_t VEC_SIZE>
void base_tape<T>::forward_interpret_from_to(
     derivative_vector_t<T, VEC_SIZE>& tangents, std::size_t from,
     std::size_t to) const {

   assert(from >= 0);
   assert(to < m_nodes.size());
   assert(from <= to);

   for (std::size_t dependent_pos = from; dependent_pos <= to;
        ++dependent_pos) {
      for (const auto& [dependee_pos, partial] : m_nodes[dependent_pos].edges) {
         tangents[derivative_idx(dependent_pos)] +=
              partial * tangents[derivative_idx(dependee_pos)];
      }
   }
}

/******************************************************************************
 * @brief Forward interpret the tape using a given tangent vector.
 * @param[inout] tangents Seeded tangent vector.
 ******************************************************************************/
template<typename T>
template<size_t VEC_SIZE>
void base_tape<T>::forward_interpret(
     derivative_vector_t<T, VEC_SIZE>& tangents) const {
   forward_interpret_from_to<VEC_SIZE>(tangents, 0, m_nodes.size() - 1);
}

/******************************************************************************
 * @brief Forward interpret the tape using a given tangent vector.
 * @param[inout] tangents Seeded tangent vector.
 ******************************************************************************/
template<typename T>
template<size_t VEC_SIZE>
void base_tape<T>::interpret_from_to(
     derivative_vector_t<T, VEC_SIZE>& derivatives, std::size_t from,
     std::size_t to) const {

   assert(from >= 0);
   assert(to < m_nodes.size());

   if (from <= to) {
      forward_interpret_from_to<VEC_SIZE>(derivatives, from, to);
   } else {
      reverse_interpret_from_to<VEC_SIZE>(derivatives, from, to);
   }
}

/******************************************************************************
 * @brief Reset the tape.
 ******************************************************************************/
template<typename T>
void base_tape<T>::reset() {
   m_nodes.clear();
   m_independent_variables.clear();
   m_dependent_variables.clear();
}

/******************************************************************************
 * @brief Create dot graph representation of the graph.
 * @param[in] filename Name of the dot file.
 ******************************************************************************/
// void base_tape::todot(const std::string& filename) const {
//    std::ofstream out(filename);
//    out << "digraph {\nrankdir=LR\n";
//    int i = dependencies.size() - 1;
//    while (i >= 0) {
//       out << dependencies[i--] << ";\n";
//       if (i > num_indeps) {
//          i -= dependencies[i] + 1;
//       }
//    }
//    i = dependencies.size() - 1;
//    while (i >= num_indeps) {
//       int j = 1;
//       for (; j <= dependencies[i - 1]; j++) {
//          out << dependencies[i - 1 - j] << "->" << dependencies[i] << ";\n";
//       }
//       i -= j + 1;
//    }
//    out << '}' << std::endl;
// }

/******************************************************************************
 * @brief Print partial derivatives and dependencies.
 ******************************************************************************/
// void base_tape::print() const {
//    std::cerr << "dependencies:\n";
//    for (const auto& e : dependencies) {
//       std::cerr << e << ' ';
//    }
//    std::cerr << "\nderivatives:\n";
//    for (const auto& e : derivatives) {
//       std::cerr << e << ' ';
//    }
//    std::cerr << std::endl << std::endl;
// }

/******************************************************************************
 * @brief Print the size of the tape as well as the adjoint vector.
 ******************************************************************************/
// void base_tape::print_size() const {
//    std::cerr << "Size of dependencies: " << dependencies.size();
//    std::cerr << " (" << dependencies.size() * sizeof(int) / 1024.0 <<
//    "KB)\n";

//    std::cerr << "Size of derivatives: " << derivatives.size();
//    std::cerr << " (" << derivatives.size() * sizeof(double) / 1024.0 <<
//    "KB)\n";

//    // std::cerr << "Size of adjoint vector: " << num_adjoints();
//    // std::cerr << " (" << num_adjoints() * sizeof(double) / 1024.0 <<
//    "KB)\n";
// }

/******************************************************************************
 * @brief Empty debug output. Prints line break.
 * @param[in] stream Output stream.
 ******************************************************************************/
// void base_tape::print_debug(std::ostream& stream) const {
//    if (debug) {
//       stream << std::endl;
//    }
// }

/******************************************************************************
 * @brief Debug output.
 * @param[in] stream Output stream.
 * @param[in] value Argument which is passed to the stream.
 * @param[in] args The rest of the arguments which are passed recursively to
 *                 this function.
 ******************************************************************************/
// template<typename T, typename... Args>
// void base_tape::print_debug(std::ostream& stream, T value, Args... args)
// const {
//    if (debug) {
//       stream << value;
//       print_debug(stream, args...);
//    }
// }

}  // namespace ad

// ************************************************************************** //

#endif  // AD_BASE_TAPE_HPP_
