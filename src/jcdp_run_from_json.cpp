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

#include "jcdp/jacobian_chain.hpp"
#include "jcdp/optimizer/branch_and_bound.hpp"
#include "jcdp/optimizer/dynamic_programming.hpp"
#include "jcdp/scheduler/branch_and_bound.hpp"
#include "jcdp/scheduler/priority_list.hpp"
#include "jcdp/sequence.hpp"
#include "jcdp/util/json.hpp"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> APPLICATION <<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

extern "C" uint32_t jcdp_run_from_json(
     const char* json_str, const char* optimizer, const char* scheduler,
     uint32_t threads, uint32_t memory, uint32_t time_to_solve,
     char* result_buffer) {
   jcdp::JacobianChain chain;
   try {
      chain = jcdp::util::jacobian_chain_from_json(json_str);
      chain.init_subchains();
   } catch (const std::exception& e) {
      std::println(std::cerr, "JSON parsing error: {}", e.what());
      return 1;
   }

   if (!result_buffer) {
      std::println(std::cerr, "No result buffer provided for output.");
      return 2;
   }

   std::shared_ptr<jcdp::scheduler::PriorityListScheduler> list_scheduler =
        std::make_shared<jcdp::scheduler::PriorityListScheduler>();
   std::shared_ptr<jcdp::scheduler::BranchAndBoundScheduler> bnb_scheduler =
        std::make_shared<jcdp::scheduler::BranchAndBoundScheduler>();

   jcdp::optimizer::DynamicProgrammingOptimizer dp_solver;
   dp_solver.set_available_threads(threads);
   dp_solver.set_available_memory(memory);

   // DP Solve (always run DP first to get an upper bound)
   dp_solver.init(chain);
   jcdp::Sequence dp_seq = dp_solver.solve();
   if (std::string(scheduler) == "list") {
      list_scheduler->schedule(dp_seq, dp_solver.m_usable_threads);
   } else if (std::string(scheduler) == "bnb") {
      bnb_scheduler->schedule(dp_seq, dp_solver.m_usable_threads);
   } else {
      std::println(std::cerr, "Unknown scheduler: {}", scheduler);
      return 3;
   }

   if (std::string(optimizer) == "dp") {
      std::string json = jcdp::util::sequence_to_json(dp_seq);
      std::strcpy(result_buffer, json.c_str());
      return 0;
   } else if (std::string(optimizer) != "bnb") {
      std::println(std::cerr, "Unknown optimizer: {}", optimizer);
      return 3;
   }

   jcdp::optimizer::BranchAndBoundOptimizer bnb_solver;
   bnb_solver.set_available_threads(threads);
   bnb_solver.set_available_memory(memory);
   bnb_solver.set_timer(time_to_solve);

   if (std::string(scheduler) == "list") {
      bnb_solver.init(chain, list_scheduler);
   } else {
      bnb_solver.init(chain, bnb_scheduler);
   }

   bnb_solver.set_upper_bound(dp_seq.makespan());
   jcdp::Sequence bnb_seq = bnb_solver.solve();
   std::string json = jcdp::util::sequence_to_json(bnb_seq);
   std::strcpy(result_buffer, json.c_str());

   return 0;
}
