/******************************************************************************
 * @file jcdp.cpp
 *
 * @brief This file is part of the JCDP package. It provides an applications
 *        that generated Jacobian chains based on a given config file and runs
 *        dynamic programming, and Branch & Bound optimizers combined with
 *        a list scheduler and a Branch & Bound scheduler.
 ******************************************************************************/

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#include "jcdp/json_api.hpp"

#if defined(_OPENMP)
#include <omp.h>
#endif

#include "jcdp/jacobian_chain.hpp"
#include "jcdp/optimizer/branch_and_bound.hpp"
#include "jcdp/optimizer/dynamic_programming.hpp"
#include "jcdp/scheduler/branch_and_bound.hpp"
#include "jcdp/scheduler/priority_list.hpp"
#include "jcdp/sequence.hpp"
#include "jcdp/util/json.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> APPLICATION <<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

extern "C" {

uint32_t EMSCRIPTEN_KEEPALIVE jcdp_run_from_json(
     const char* chain_json, const char* sequence_json, const char* optimizer,
     const char* scheduler, uint32_t omp_threads, uint32_t available_threads,
     uint32_t available_memory, uint32_t time_to_solve, bool matrix_free,
     const char** result_buffer) {

   // Static buffer to hold the result. This persists between calls, so we
   // don't need to malloc/free manually from JS.
   static std::string g_result_json;

#if defined(_OPENMP)
   omp_set_num_threads(omp_threads);
#endif

   // Clear previous result
   g_result_json.clear();
   if (result_buffer) {
      *result_buffer = nullptr;
   }

   jcdp::JacobianChain chain;
   jcdp::Sequence partial_sequence;
   try {
      chain = jcdp::util::jacobian_chain_from_json(chain_json);
      chain.init_subchains();
      partial_sequence = jcdp::util::sequence_from_json(sequence_json);
   } catch (const std::exception& e) {
      std::println(std::cerr, "JSON parsing error: {}", e.what());
      return 1;
   }

   std::shared_ptr<jcdp::scheduler::PriorityListScheduler> list_scheduler =
        std::make_shared<jcdp::scheduler::PriorityListScheduler>();
   std::shared_ptr<jcdp::scheduler::BranchAndBoundScheduler> bnb_scheduler =
        std::make_shared<jcdp::scheduler::BranchAndBoundScheduler>();

   jcdp::optimizer::DynamicProgrammingOptimizer dp_solver;
   dp_solver.set_available_threads(available_threads);
   dp_solver.set_available_memory(available_memory);
   dp_solver.set_matrix_free(matrix_free);
   dp_solver.set_group_consecutive_eliminations(false);

   // DP Solve (always run DP first to get an upper bound)
   dp_solver.init(chain);
   jcdp::Sequence dp_seq = dp_solver.solve(partial_sequence);
   if (std::string(scheduler) == "list") {
      list_scheduler->schedule(dp_seq, dp_solver.m_usable_threads);
   } else if (std::string(scheduler) == "bnb") {
      bnb_scheduler->schedule(dp_seq, dp_solver.m_usable_threads);
   } else if (std::string(scheduler) != "none") {
      std::println(std::cerr, "Unknown scheduler: {}", scheduler);
      return 2;
   }

   if (std::string(optimizer) == "dp") {
      // Just return the DP solution
      g_result_json = jcdp::util::sequence_to_json(dp_seq);
      if (result_buffer) {
         *result_buffer = g_result_json.c_str();
      }
      return 0;
   } else if (std::string(optimizer) != "bnb") {
      std::println(std::cerr, "Unknown optimizer: {}", optimizer);
      return 3;
   }

   jcdp::optimizer::BranchAndBoundOptimizer bnb_solver;
   bnb_solver.set_available_threads(available_threads);
   bnb_solver.set_available_memory(available_memory);
   bnb_solver.set_timer(time_to_solve);
   bnb_solver.set_matrix_free(matrix_free);
   bnb_solver.set_group_consecutive_eliminations(false);

   if (std::string(scheduler) == "list") {
      bnb_solver.init(chain, list_scheduler);
   } else if (std::string(scheduler) == "bnb") {
      bnb_solver.init(chain, bnb_scheduler);
   } else {
      std::println(std::cerr, "Invalid scheduler: {}", scheduler);
      return 4;
   }

   bnb_solver.set_upper_bound(dp_seq.makespan());
   jcdp::Sequence bnb_seq = bnb_solver.solve(partial_sequence);

   g_result_json = jcdp::util::sequence_to_json(bnb_seq);
   if (result_buffer) {
      *result_buffer = g_result_json.c_str();
   }

   return 0;
}
}
