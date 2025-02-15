#include <filesystem>
#include <iostream>
#include <memory>

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

   std::filesystem::path output_file = "./results.csv";
   if (argc > 2) {
      output_file = std::filesystem::path(argv[2]);
   }

   std::ofstream out(output_file);
   if (!out) {
      std::cerr << "Failed to open " << output_file << std::endl;
      return -1;
   }

   jcdp::JacobianChain chain;
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
         jcdp::Sequence bnb_seq = bnb_solver.solve();

         out << bnb_seq.makespan() << ",";
         out << bnb_seq_list.makespan() << ",";
         out << static_cast<double>(bnb_seq.makespan()) /
                     static_cast<double>(bnb_seq_list.makespan())
             << ",";
         out << dp_makespan << ",";
         out << static_cast<double>(bnb_seq.makespan()) /
                     static_cast<double>(dp_makespan)  << ",";
         out << dp_seq.makespan() << ",";
         out << static_cast<double>(dp_seq.makespan()) /
                     static_cast<double>(dp_makespan);

         if (threads < chain.length()) {
            out << ",";
         }
      }

      out << std::endl;
   }

   out.close();
   return 0;
}
