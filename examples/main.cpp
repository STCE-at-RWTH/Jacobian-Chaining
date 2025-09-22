// ************************************************************************** //
//           Advanced Algorithmic Differentiation SS23 - Tutorial 5           //
//                                                                            //
//                        Linear Algebra & Vector mode                        //
//                                                                            //
// Copyright (C) 2023 Software and Tools for Computational Engineering (STCE) //
//               RWTH Aachen University - www.stce.rwth-aachen.de             //
// ************************************************************************** //

// 100MB Eigen vectors
#define EIGEN_STACK_ALLOCATION_LIMIT 104857600

#include "ad/ad.hpp"
#include <Eigen/Dense>

#include <tuple>
#include <limits>
#include <cmath>
#include <chrono>

// Whether to print debug tape informations or perform benchmark
constexpr bool debug = true;

// TODO: Exercise 3
// Derivative vector size
constexpr size_t vector_size = 1;

// *********************** Euler-Maruyama Parameters ************************ //

// Number of steps in the Euler-Maruyama scheme
constexpr size_t N_steps = debug ? 1 : 1000;

// Time interval we want to solve: [0, T]
constexpr double T = 1.0;

// Time step
constexpr double dt = T / N_steps;

// ****************************** Custom Types ****************************** //

template <typename T>
using vec_N = Eigen::Matrix<T, N_steps, 1>;

template <typename T>
using vec_t = ad::vector_t<T, vector_size>;

template <typename T>
using der_vec_t = ad::derivative_vector_t<T, vector_size>;

// ************************************************************************** //

// Function f in the SDE
template<typename TX, typename TP, typename TT>
auto f(const size_t i, const TX& x, const TP& p, const TT& t) {
  return p * sin(x*t);
}

// Function g in the SDE
template<typename TX, typename TP, typename TT>
auto g(const size_t i, const TX& x, const TP& p, const TT& t) {
  return p * cos(x*t);
}

// Calculate a single path
template<typename AT, typename PT>
AT single_path(const AT& x0, const AT& p, const vec_N<PT>& dW) {

  // Init state variables
  PT t = 0;
  AT x = x0;

  // Perform steps
  for (size_t i = 0; i < N_steps; i += 1) {
    x = x + f(i, x, p, t) * dt + g(i, x, p, t) * dW(i);
    t += dt;
  }

  return x;

}

// ************************************************************************** //

// Finite differences driver for given function
template<typename PT>
std::tuple<PT, vec_t<PT>> fd_driver(const PT& x_p, const PT& p_p,
                                      const vec_N<PT>& dW) {

  // Choose optimal h and round it to the nearest representable number
  double h = std::cbrt(std::numeric_limits<double>::epsilon());
  h *= (1.0 + abs(p_p));
  h = std::pow(2.0, std::round(std::log(h) / std::log(2.0)));

  vec_t<PT> x = vec_t<PT>(x_p);
  vec_t<PT> p_plus = vec_t<PT>(p_p + h);
  vec_t<PT> p_minus = vec_t<PT>(p_p - h);

  // Calculate and return passive value and central finite differences
  return {
    single_path(x_p, p_p, dW),
    (single_path(x, p_plus, dW) - single_path(x, p_minus, dW)) / (2.0 * h)};

}

// Tangent driver for given function
template<typename PT>
std::tuple<PT, vec_t<PT>> tangent_driver(const PT& x_p, const PT& p_p,
                                           const vec_N<PT>& dW) {

  using tangent_t = ad::noet::tangent<PT, vector_size>;

  // Init tangent variables
  tangent_t xt(x_p);
  tangent_t pt(p_p);
  pt.derivative = vec_t<PT>(1.0);

  // Calculate and return passive value and derivative
  tangent_t yt = single_path(xt, pt, dW);
  return {yt.value, yt.derivative};

}

// Adjoint driver for given function
template<typename PT>
std::tuple<PT, vec_t<PT>> taped_adjoint_driver(const PT& x_p, const PT& p_p,
                                                 const vec_N<PT>& dW) {

  using adjoint_t = ad::noet::active_t<PT>;
  using tape_t = ad::noet::tape<PT>;

  // TODO: Exercise 3
  // using adjoint_t = ad::noet::adjoint<PT, vector_size>;
  // using tape_t = ad::noet::tape<PT, vector_size>;

  // Get reference to global tape and make sure it is clean
  tape_t &t = adjoint_t::global_tape;
  t.reset();

  // Init adjoint variables
  adjoint_t xa(x_p);
  xa.register_input();
  adjoint_t pa(p_p);
  pa.register_input();

   std::cout << xa << "" << pa << std::endl;

  // Record tape and create dot file
  adjoint_t ya = single_path(xa, pa, dW);

   std::cout << ya << std::endl;

  // Allocate vector of adjoints, seed it and interpret the tape
  der_vec_t<PT> adjoints = der_vec_t<PT>::Zero(t.derivative_vector_size(), vector_size);
  adjoints.row(t.derivative_idx(ya.tape_index())) = vec_t<PT>(1.0);
  t.template reverse_interpret<vector_size>(adjoints);

  // Harvest and return passive value and derivative
  return {ya.value(), adjoints.row(t.derivative_idx(pa.tape_index()))};

}

// // Banded adjoint driver for given function
// template<typename PT>
// std::tuple<PT, vec_t<PT>> banded_adjoint_driver(const PT& x_p,
//                                                   const PT& p_p,
//                                                   const vec_N<PT>& dW) {

//   using adjoint_t = ad::noet::banded_adjoint;
//   using tape_t = ad::noet::banded_tape;

//   // TODO: Exercise 3
//   // using adjoint_t = ad::noet::banded_adjoint<PT, vector_size>;
//   // using tape_t = ad::noet::banded_tape<PT, vector_size>;

//   // Get reference to global tape and make sure it is clean
//   tape_t &t = adjoint_t::global_tape;
//   t.num_indeps = 2;
//   t.num_deps = 1;
//   t.reset();

//   // Init banded adjoint variables
//   adjoint_t xa(x_p);
//   xa.register_input();
//   adjoint_t pa(p_p);
//   pa.register_input();

//   // Record tape and create dot file
//   adjoint_t ya = single_path(xa, pa, dW);
//   if constexpr (debug) {
//     t.todot("banded_tape.dot");
//     t.print();
//     t.print_size();
//   }

//   // Allocate vector of adjoints, seed it and interpret the tape
//   der_vec_t<PT> adjoints = der_vec_t<PT>::Zero(t.num_adjoints(), vector_size);
//   adjoints.row(t.derivative_idx(ya.dep_pos)) = vec_t<PT>(1.0);
//   t.interpret(adjoints);

//   // Harvest and return passive value and derivative
//   return {ya.value, adjoints.row(t.derivative_idx(pa.dep_pos))};

// }

// // Dedicated adjoint driver for given function
// template<typename PT>
// std::tuple<PT, vec_t<PT>> dedicated_adjoint_driver(const PT& x_p,
//                                                      const PT& p_p,
//                                                      const vec_N<PT>& dW) {

//   using adjoint_t = ad::noet::dedicated_adjoint;
//   using tape_t = ad::noet::dedicated_tape;

//   // TODO: Exercise 3
//   // using adjoint_t = ad::noet::dedicated_adjoint<PT, vector_size>;
//   // using tape_t = ad::noet::dedicated_tape<PT, vector_size>;

//   // Get reference to global tape and make sure it is clean
//   tape_t &t = adjoint_t::global_tape;
//   t.num_indeps = 2;
//   t.num_deps = 1;
//   t.reset();

//   // Init dedicated adjoint variables
//   adjoint_t xa(x_p);
//   xa.register_input();
//   adjoint_t pa(p_p);
//   pa.register_input();

//   // Record tape and create dot file
//   adjoint_t ya = single_path(xa, pa, dW);
//   if constexpr (debug) {
//     t.todot("dedicated_tape.dot");
//     t.print();
//     t.print_size();
//   }

//   // Allocate vector of adjoints, seed it and interpret the tape
//   der_vec_t<PT> adjoints = der_vec_t<PT>::Zero(t.num_adjoints(), vector_size);
//   adjoints.row(t.derivative_idx(ya.dep_pos)) = vec_t<PT>(1.0);
//   t.interpret(adjoints);

//   // Harvest and return passive value and derivative
//   return {ya.value, adjoints.row(t.derivative_idx(pa.dep_pos))};

// }

// // Expression template adjoint driver for given function
// template<typename PT>
// std::tuple<PT, vec_t<PT>> et_adjoint_driver(const PT& x_p, const PT& p_p,
//                                      const vec_N<PT>& dW) {

//   using adjoint_t = ad::et::adjoint<PT>;
//   using tape_t = ad::et::tape;

//   // TODO: Exercise 3
//   // using adjoint_t = ad::et::adjoint<PT, vector_size>;
//   // using tape_t = ad::et::tape<PT, vector_size>;

//   // Get reference to global tape and make sure it is clean
//   tape_t &t = adjoint_t::global_tape;
//   t.num_indeps = 2;
//   t.num_deps = 1;
//   t.reset();

//   // Init dedicated adjoint variables
//   adjoint_t xa(x_p);
//   xa.register_input();
//   adjoint_t pa(p_p);
//   pa.register_input();

//   // Record tape and create dot file
//   adjoint_t ya = single_path(xa, pa, dW);
//   if constexpr (debug) {
//     t.todot("tape.dot");
//     t.print();
//     t.print_size();
//   }

//   // Allocate vector of adjoints, seed it and interpret the tape
//   der_vec_t<PT> adjoints = der_vec_t<PT>::Zero(t.num_adjoints(), vector_size);
//   adjoints.row(t.derivative_idx(ya.dep_pos)) = vec_t<PT>(1.0);
//   t.interpret(adjoints);

//   // Harvest and return passive value and derivative
//   return {ya.value, adjoints.row(t.derivative_idx(pa.dep_pos))};

// }

// // Expression template tangent driver for given function
// template<typename PT>
// std::tuple<PT, vec_t<PT>> et_tangent_driver(const PT& x_p, const PT& p_p,
//                                               const vec_N<PT>& dW) {

//   using tangent_t = ad::et::tangent<PT, vector_size>;

//   // Init tangent variables
//   tangent_t xt(x_p);
//   tangent_t pt(p_p);
//   pt.derivative = PT(1.0);

//   // Calculate and return passive value and derivative
//   tangent_t yt = single_path(xt, pt, dW);
//   return {yt.value, yt.derivative};

// }

// ************************************************************************** //

// Main entry point
int main(int c, char* v[]) {

  using std::abs;

  // Passive type
  using PT = double;

  // Starting value and parameter vector
  PT x0 = 1.0;
  PT p = 4.2;

  // Generate N_steps randomly distributed variables
  vec_N<PT> dW = std::sqrt(dt) * (vec_N<PT>::Random() + vec_N<PT>::Constant(1.0));

  // Timing results
  Eigen::ArrayXd runtime(debug ? 1 : 100);

  // Average runtime
  auto mean = [&runtime]() -> double {
    return runtime.mean();
  };

  // Runtime standard deviation
  auto stddev = [&runtime]() -> double {
    return std::sqrt((runtime - runtime.mean()).square().sum() / (runtime.size() - 1));
  };

  // -------------------------- Finite Differences -------------------------- //

  PT X_fd;
  vec_t<PT> dXdp_fd;
  for (size_t i = 0; i < runtime.size(); i++) {
    auto start = std::chrono::high_resolution_clock::now();
    std::tie(X_fd, dXdp_fd) = fd_driver(x0, p, dW);
    runtime(i) = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - start).count();
  }

  std::cout << "Finite differences runtime: " << mean() << "μs ";
  std::cout << "(stddev = " << stddev() << "μs)\n";
  if constexpr (debug) {
    std::cout << "g from finite differences driver: " << X_fd << "\n";
    std::cout << "dXdp from finite differences driver: " << dXdp_fd << "\n\n";
  }

  // ------------------------------- Tangent -------------------------------- //

  PT X_t;
  vec_t<PT> dXdp_t;
  for (size_t i = 0; i < runtime.size(); i++) {
    auto start = std::chrono::high_resolution_clock::now();
    std::tie(X_t, dXdp_t) = tangent_driver(x0, p, dW);
    runtime(i) = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - start).count();
  }

  std::cout << "Tangent runtime: " << mean() << "μs ";
  std::cout << "(stddev = " << stddev() << "μs)\n";
  if constexpr (debug) {
    std::cout << "g from tangent driver: " << X_t;
    std::cout << " (error = " << abs(X_fd - X_t) << ")\n";
    std::cout << "dXdp from tangent driver: " << dXdp_t;
    std::cout << " (fd error = " << abs(dXdp_fd - dXdp_t) << ")\n\n";
  }

  // ------------------------------- Adjoint -------------------------------- //

  PT X_a;
  vec_t<PT> dXdp_a;
  for (size_t i = 0; i < runtime.size(); i++) {
    auto start = std::chrono::high_resolution_clock::now();
    std::tie(X_a, dXdp_a) = taped_adjoint_driver(x0, p, dW);
    runtime(i) = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - start).count();
  }

  std::cout << "\nAdjoint runtime: " << mean() << "μs ";
  std::cout << "(stddev = " << stddev() << "μs)\n";
  if constexpr (debug) {
    std::cout << "g from adjoint driver: " << X_a;
    std::cout << " (error = " << abs(X_fd - X_a) << ")\n";
    std::cout << "dXdp from adjoint driver: " << dXdp_a;
    std::cout << " (fd error = " << abs(dXdp_fd - dXdp_a) << ")\n\n";
  }

//   // ---------------------------- Banded Adjoint ---------------------------- //

//   PT X_ba;
//   vec_t<PT> dXdp_ba;
//   for (size_t i = 0; i < runtime.size(); i++) {
//     auto start = std::chrono::high_resolution_clock::now();
//     std::tie(X_ba, dXdp_ba) = banded_adjoint_driver(x0, p, dW);
//     runtime(i) = std::chrono::duration_cast<std::chrono::microseconds>(
//         std::chrono::high_resolution_clock::now() - start).count();
//   }

//   std::cout << "\nBanded adjoint runtime: " << mean() << "μs ";
//   std::cout << "(stddev = " << stddev() << "μs)\n";
//   if constexpr (debug) {
//     std::cout << "g from banded adjoint driver: " << X_ba;
//     std::cout << " (error = " << abs(X_fd - X_ba) << ")\n";
//     std::cout << "dXdp from banded adjoint driver: " << dXdp_ba;
//     std::cout << " (fd error = " << abs(dXdp_fd - dXdp_ba) << ")\n\n";
//   }

//   // -------------------------- Dedicated Adjoint --------------------------- //

//   PT X_da;
//   vec_t<PT> dXdp_da;
//   for (size_t i = 0; i < runtime.size(); i++) {
//     auto start = std::chrono::high_resolution_clock::now();
//     std::tie(X_da, dXdp_da) = dedicated_adjoint_driver(x0, p, dW);
//     runtime(i) = std::chrono::duration_cast<std::chrono::microseconds>(
//         std::chrono::high_resolution_clock::now() - start).count();
//   }

//   std::cout << "\nDedicated adjoint runtime: " << mean() << "μs ";
//   std::cout << "(stddev = " << stddev() << "μs)\n";
//   if constexpr (debug) {
//     std::cout << "g from dedicated adjoint driver: " << X_da;
//     std::cout << " (error = " << abs(X_fd - X_da) << ")\n";
//     std::cout << "dXdp from dedicated adjoint driver: " << dXdp_da;
//     std::cout << " (fd error = " << abs(dXdp_fd - dXdp_da) << ")\n\n";
//   }

//   // --------------------- Expression Template Adjoint ---------------------- //

//   PT X_eta;
//   vec_t<PT> dXdp_eta;
//   for (size_t i = 0; i < runtime.size(); i++) {
//     auto start = std::chrono::high_resolution_clock::now();
//     std::tie(X_eta, dXdp_eta) = et_adjoint_driver(x0, p, dW);
//     runtime(i) = std::chrono::duration_cast<std::chrono::microseconds>(
//         std::chrono::high_resolution_clock::now() - start).count();
//   }

//   std::cout << "\nExpression template adjoint runtime: " << mean() << "μs ";
//   std::cout << "(stddev = " << stddev() << "μs)\n";
//   if constexpr (debug) {
//     std::cout << "g from expression template adjoint driver: " << X_eta;
//     std::cout << " (error = " << abs(X_fd - X_eta) << ")\n";
//     std::cout << "dXdp from expression template adjoint driver: " << dXdp_eta;
//     std::cout << " (fd error = " << abs(dXdp_fd - dXdp_eta) << ")\n\n";
//   }

//   // --------------------- Expression Template Tangent ---------------------- //

//   PT X_ett;
//   vec_t<PT> dXdp_ett;
//   for (size_t i = 0; i < runtime.size(); i++) {
//     auto start = std::chrono::high_resolution_clock::now();
//     std::tie(X_ett, dXdp_ett) = et_tangent_driver(x0, p, dW);
//     runtime(i) = std::chrono::duration_cast<std::chrono::microseconds>(
//         std::chrono::high_resolution_clock::now() - start).count();
//   }

//   std::cout << "\nExpression template tangent runtime: " << mean() << "μs ";
//   std::cout << "(stddev = " << stddev() << "μs)\n";
//   if constexpr (debug) {
//     std::cout << "g from expression template tangent driver: " << X_ett;
//     std::cout << " (error = " << abs(X_fd - X_ett) << ")\n";
//     std::cout << "dXdp from expression template tangent driver: " << dXdp_ett;
//     std::cout << " (fd error = " << abs(dXdp_fd - dXdp_ett) << ")\n\n";
//   }

  return 0;

}

