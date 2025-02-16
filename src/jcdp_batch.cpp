#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

#include "jcdp/generator.hpp"
#include "jcdp/jacobian_chain.hpp"
#include "jcdp/optimizer/branch_and_bound.hpp"
#include "jcdp/optimizer/dynamic_programming.hpp"
#include "jcdp/scheduler/branch_and_bound.hpp"
#include "jcdp/scheduler/priority_list.hpp"

int main(int argc, char* argv[]) {
   jcdp::JacobianChainGenerator jcgen;
   jcdp::optimizer::DynamicProgrammingOptimizer dp_solver;
   jcdp::optimizer::BranchAndBoundOptimizer bnb_solver;

   std::shared_ptr<jcdp::scheduler::BranchAndBoundScheduler> bnb_scheduler =
        std::make_shared<jcdp::scheduler::BranchAndBoundScheduler>();
   std::shared_ptr<jcdp::scheduler::PriorityListScheduler> list_scheduler =
        std::make_shared<jcdp::scheduler::PriorityListScheduler>();

   if (argc < 2) {
      jcgen.print_help(std::cout);
      dp_solver.print_help(std::cout);
      return -1;
   }

   const std::filesystem::path config_filename(argv[1]);
   try {
      dp_solver.parse_config(config_filename, true);
      bnb_solver.parse_config(config_filename, true);
      jcgen.parse_config(config_filename, true);
      jcgen.init_rng();
   } catch (const std::runtime_error& bcfe) {
      std::cerr << bcfe.what() << std::endl;
      return -1;
   }

   std::string output_file_name = "results";
   if (argc > 2) {
      output_file_name = argv[2];
   }

   jcdp::JacobianChain chain;
   while (!jcgen.empty()) {
      std::filesystem::path output_file = (
         output_file_name + std::to_string(jcgen.current_length()) + ".csv");

      std::ofstream out(output_file);
      if (!out) {
         std::cerr << "Failed to open " << output_file << std::endl;
         return -1;
      }

      for (std::size_t t = 1; t <= jcgen.current_length(); ++t) {
         out << "BnB_BnB/" << t << "/finished,";
         out << "BnB_BnB/" << t << ",";
         out << "BnB_List/" << t << ",";
         out << "DP/" << t << ",";
         out << "DP_BnB/" << t << ",";

         if (t < jcgen.current_length()) {
            out << ",";
         } else {
            out << "\n";
         }
      }

      while (jcgen.next(chain)) {
         chain.init_subchains();

         // Solve via dynamic programming
         dp_solver.init(chain);
         dp_solver.m_usable_threads = chain.length();
         dp_solver.solve();

         for (std::size_t threads = 1; threads <= chain.length(); ++threads) {
            jcdp::Sequence dp_seq = dp_solver.get_sequence(threads);
            const std::size_t dp_makespan = dp_seq.makespan();

            // Schedule dynamic programming sequence via branch & bound
            bnb_scheduler->schedule(dp_seq, threads, dp_makespan);

            // Solve via branch & bound + List scheduling
            bnb_solver.init(chain, list_scheduler);
            bnb_solver.set_upper_bound(dp_seq.makespan());
            bnb_solver.m_usable_threads = threads;
            jcdp::Sequence bnb_seq_list = bnb_solver.solve();

            // Solve via branch & bound
            bnb_solver.init(chain, bnb_scheduler);
            bnb_solver.m_usable_threads = threads;
            jcdp::Sequence bnb_seq = bnb_solver.solve();

            out << bnb_solver.finished_in_time() << ",";
            out << bnb_seq.makespan() << ",";
            out << bnb_seq_list.makespan() << ",";
            out << dp_makespan << ",";
            out << dp_seq.makespan();

            if (threads < chain.length()) {
               out << ",";
            }
         }

         out << std::endl;
      }

      out.close();
   }

   return 0;
}
