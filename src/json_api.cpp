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
#include <map>
#include <memory>
#include <mutex>

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

// Global storage for results to allow concurrent calls
static std::map<int32_t, std::shared_ptr<jcdp::SolverState>> g_states;
static int32_t g_next_handle = 1;
static std::mutex g_results_mutex;

extern "C" {

/**
 * @brief Frees the result string associated with the given handle.
 *
 * @param handle The handle of the result to free.
 */
void EMSCRIPTEN_KEEPALIVE jcdp_free_result(int32_t handle) {
   std::lock_guard<std::mutex> lock(g_results_mutex);
   g_states.erase(handle);
}

int32_t EMSCRIPTEN_KEEPALIVE jcdp_init() {
   std::lock_guard<std::mutex> lock(g_results_mutex);
   int32_t new_handle = g_next_handle++;
   g_states[new_handle] = std::make_shared<jcdp::SolverState>();
   return new_handle;
}

/**
 * @brief Gets the raw pointer to the SolverState for the given handle.
 *
 * @param handle The handle of the execution.
 * @return Pointer to SolverState, or nullptr if not found.
 */
void* EMSCRIPTEN_KEEPALIVE jcdp_get_state_ptr(int32_t handle) {
   std::lock_guard<std::mutex> lock(g_results_mutex);
   auto it = g_states.find(handle);
   if (it != g_states.end()) {
      return it->second.get();
   }
   return nullptr;
}

int32_t EMSCRIPTEN_KEEPALIVE jcdp_run_from_json(
     int32_t handle,
     const char* chain_json, const char* sequence_json, const char* optimizer,
     const char* scheduler, uint32_t omp_threads, uint32_t available_threads,
     uint32_t available_memory, uint32_t time_to_solve, bool matrix_free) {

#if defined(_OPENMP)
   omp_set_num_threads(omp_threads);
#endif

   int32_t current_handle = handle;
   std::shared_ptr<jcdp::SolverState> current_state = nullptr;

   {
      std::lock_guard<std::mutex> lock(g_results_mutex);
      // If no valid handle provided, create a new one
      if (current_handle == 0) {
         current_handle = g_next_handle++;
         g_states[current_handle] = std::make_shared<jcdp::SolverState>();
      } else {
         // Verify handle exists
         if (g_states.find(current_handle) == g_states.end()) {
            g_next_handle = std::max(g_next_handle, current_handle + 1);
            g_states[current_handle] = std::make_shared<jcdp::SolverState>();
         }
      }
      current_state = g_states[current_handle];
   }

   jcdp::JacobianChain chain;
   jcdp::Sequence partial_sequence;
   try {
      chain = jcdp::util::jacobian_chain_from_json(chain_json);
      chain.init_subchains();
      partial_sequence = jcdp::util::sequence_from_json(sequence_json);
   } catch (const std::exception& e) {
      std::println(std::cerr, "JSON parsing error: {}", e.what());
      return -1;
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
      return -2;
   }

   if (std::string(optimizer) == "dp") {
      // Just return the DP solution
      current_state->optimal_sequence = dp_seq;
      current_state->optimal_sequence_json =
           jcdp::util::sequence_to_json(dp_seq);
      current_state->result_ptr =
           current_state->optimal_sequence_json.c_str();
      current_state->state = jcdp::StateControl::DONE;
   } else if (std::string(optimizer) == "bnb") {
      jcdp::optimizer::BranchAndBoundOptimizer bnb_solver;
      bnb_solver.set_available_threads(available_threads);
      bnb_solver.set_available_memory(available_memory);
      bnb_solver.set_matrix_free(matrix_free);
      bnb_solver.set_timer(time_to_solve);
      bnb_solver.set_group_consecutive_eliminations(false);

      if (std::string(scheduler) == "list") {
         bnb_solver.init(chain, list_scheduler, current_state);
      } else if (std::string(scheduler) == "bnb") {
         bnb_solver.init(chain, bnb_scheduler, current_state);
      } else {
         std::println(std::cerr, "Invalid scheduler for BnB: {}", scheduler);
         return -4;
      }

      bnb_solver.set_upper_bound(dp_seq.makespan());
      jcdp::Sequence bnb_seq = bnb_solver.solve(partial_sequence);
      current_state->optimal_sequence = bnb_seq;
   } else {
      std::println(std::cerr, "Unknown optimizer: {}", optimizer);
      return -3;
   }

   return 0;
}

}  // extern "C"
