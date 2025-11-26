/******************************************************************************
 * @file jcdp.cpp
 *
 * @brief This file is part of the JCDP package. It provides an applications
 *        that generated Jacobian chains based on a given config file and runs
 *        dynamic programming, and Branch & Bound optimizers combined with
 *        a list scheduler and a Branch & Bound scheduler.
 ******************************************************************************/

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <chrono>
#include <iostream>
#include <memory>
#include <cstdint>
#include <omp.h>

#include "jcdp/jacobian_chain.hpp"
#include "jcdp/optimizer/branch_and_bound.hpp"
#include "jcdp/optimizer/dynamic_programming.hpp"
#include "jcdp/scheduler/branch_and_bound.hpp"
#include "jcdp/scheduler/priority_list.hpp"
#include "jcdp/sequence.hpp"
#include "jcdp/util/json.hpp"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> APPLICATION <<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

extern "C" {

uint32_t jcdp_run_dp_bnb_from_json(char* json_str, uint32_t threads, uint32_t memory) {
   jcdp::JacobianChain chain;
   try {
      chain = jcdp::util::jacobian_chain_from_json(json_str);
      chain.init_subchains();
   } catch (const std::exception& e) {
      std::println(std::cerr, "JSON parsing error: {}", e.what());
      return false;
   }

   jcdp::optimizer::DynamicProgrammingOptimizer dp_solver;
   dp_solver.set_available_threads(threads);
   dp_solver.set_available_memory(memory);

   std::shared_ptr<jcdp::scheduler::BranchAndBoundScheduler> bnb_scheduler =
        std::make_shared<jcdp::scheduler::BranchAndBoundScheduler>();

   // DP Solve
   dp_solver.init(chain);
   jcdp::Sequence dp_seq = dp_solver.solve();

   // BnB Schedule
   bnb_scheduler->schedule(dp_seq, dp_solver.m_usable_threads);

   return 0;
}

uint32_t jcdp_run_bnb_list_from_json(char* json_str, uint32_t threads, uint32_t memory, uint32_t time_to_solve) {
   jcdp::JacobianChain chain;
   try {
      chain = jcdp::util::jacobian_chain_from_json(json_str);
      chain.init_subchains();
   } catch (const std::exception& e) {
      std::println(std::cerr, "JSON parsing error: {}", e.what());
      return false;
   }

   // Run DP for Upper Bound
   jcdp::optimizer::DynamicProgrammingOptimizer dp_solver;
   dp_solver.set_available_threads(threads);
   dp_solver.set_available_memory(memory);
   dp_solver.init(chain);
   jcdp::Sequence dp_seq = dp_solver.solve();

   // BnB Solve with List Scheduler
   jcdp::optimizer::BranchAndBoundOptimizer bnb_solver;
   bnb_solver.set_available_threads(threads);
   bnb_solver.set_available_memory(memory);
   bnb_solver.set_timer(time_to_solve);

   std::shared_ptr<jcdp::scheduler::PriorityListScheduler> scheduler =
        std::make_shared<jcdp::scheduler::PriorityListScheduler>();

   bnb_solver.init(chain, scheduler);
   bnb_solver.set_upper_bound(dp_seq.makespan());

   jcdp::Sequence bnb_seq = bnb_solver.solve();

   return 0;
}

uint32_t jcdp_run_bnb_bnb_from_json(char* json_str, uint32_t threads, uint32_t memory, uint32_t time_to_solve) {
   jcdp::JacobianChain chain;
   try {
      chain = jcdp::util::jacobian_chain_from_json(json_str);
      chain.init_subchains();
   } catch (const std::exception& e) {
      std::println(std::cerr, "JSON parsing error: {}", e.what());
      return false;
   }

   // Run DP for Upper Bound
   jcdp::optimizer::DynamicProgrammingOptimizer dp_solver;
   dp_solver.set_available_threads(threads);
   dp_solver.set_available_memory(memory);
   dp_solver.init(chain);
   jcdp::Sequence dp_seq = dp_solver.solve();

   // BnB Solve with List Scheduler
   jcdp::optimizer::BranchAndBoundOptimizer bnb_solver;
   bnb_solver.set_available_threads(threads);
   bnb_solver.set_available_memory(memory);
   bnb_solver.set_timer(time_to_solve);

   std::shared_ptr<jcdp::scheduler::BranchAndBoundScheduler> scheduler =
        std::make_shared<jcdp::scheduler::BranchAndBoundScheduler>();

   bnb_solver.init(chain, scheduler);
   bnb_solver.set_upper_bound(dp_seq.makespan());

   jcdp::Sequence bnb_seq = bnb_solver.solve();

   return 0;
}

uint32_t jcdp_run_from_json(const char* json_str, const char* optimizer, const char* scheduler, uint32_t threads, uint32_t memory, uint32_t time_to_solve) {
   jcdp::JacobianChain chain;
   try {
      chain = jcdp::util::jacobian_chain_from_json(json_str);
      chain.init_subchains();
   } catch (const std::exception& e) {
      std::println(std::cerr, "JSON parsing error: {}", e.what());
      return 1;
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
      return 2;
   }

   if (std::string(optimizer) == "dp") {
      // Just return the DP solution
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

   return 0;
}

}