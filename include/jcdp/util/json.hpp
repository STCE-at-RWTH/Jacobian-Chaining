/******************************************************************************
 * @file jcdp/util/json.hpp
 *
 * @brief This file is part of the JCDP package. It provides a utility function
 *        that creates a Jacobian chain from a serialized JSON string in the
 *        JSON graph format.
 ******************************************************************************/

#ifndef JCDP_UTIL_JSON_HPP_
#define JCDP_UTIL_JSON_HPP_

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> INCLUDES <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "jcdp/jacobian.hpp"
#include "jcdp/jacobian_chain.hpp"
#include "jcdp/operation.hpp"
#include "jcdp/sequence.hpp"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HEADER CONTENTS <<<<<<<<<<<<<<<<<<<<<<<<<<<< //

namespace jcdp::util {

/**
 * @brief Creates a JacobianChain from a serialized JSON string in the JSON
 * graph format.
 *
 * @param json_str The JSON string representing the graph.
 * @return JacobianChain The constructed Jacobian chain.
 */
inline auto jacobian_chain_from_json(const std::string& json_str)
     -> JacobianChain {
   using json = nlohmann::json;
   auto j = json::parse(json_str);

   JacobianChain chain;

   // Handle JGF (JSON Graph Format) - look for "graph" object, then "edges"
   const auto& graph = j.contains("graph") ? j["graph"] : j;

   // Parse nodes to get vector sizes
   std::map<std::string, std::size_t> node_sizes;
   if (graph.contains("nodes")) {
      for (const auto& node : graph["nodes"]) {
         std::string id;
         if (node["id"].is_string()) {
            id = node["id"].get<std::string>();
         } else {
            id = std::to_string(node["id"].get<std::size_t>());
         }

         const auto& data = node.contains("metadata") ? node["metadata"] : node;
         if (data.contains("vectorSize")) {
            node_sizes[id] = data["vectorSize"].get<std::size_t>();
         }
      }
   }

   const auto& edges = graph.contains("edges") ?
                            graph["edges"] :
                            (graph.is_array() ? graph : json::array());

   for (const auto& edge : edges) {
      Jacobian jac;
      std::string source_id;
      std::string target_id;

      // Parse source/target indices
      if (edge.contains("source")) {
         if (edge["source"].is_string()) {
            source_id = edge["source"].get<std::string>();
            jac.i = std::stoul(source_id);
         } else {
            jac.i = edge["source"].get<std::size_t>();
            source_id = std::to_string(jac.i);
         }
      }
      if (edge.contains("target")) {
         if (edge["target"].is_string()) {
            target_id = edge["target"].get<std::string>();
            jac.j = std::stoul(target_id);
         } else {
            jac.j = edge["target"].get<std::size_t>();
            target_id = std::to_string(jac.j);
         }
      }

      // Parse metadata/properties
      // Check if properties are in a "metadata" object or at the top level
      const auto& data = edge.contains("metadata") ? edge["metadata"] : edge;

      // Try to get n/m from node sizes if available
      if (node_sizes.count(source_id)) {
         jac.n = node_sizes[source_id];
      }
      if (node_sizes.count(target_id)) {
         jac.m = node_sizes[target_id];
      }

      if (data.contains("n")) {
         jac.n = data["n"].get<std::size_t>();
      }
      if (data.contains("m")) {
         jac.m = data["m"].get<std::size_t>();
      }
      if (data.contains("ku")) {
         jac.ku = data["ku"].get<std::size_t>();
      }
      if (data.contains("kl")) {
         jac.kl = data["kl"].get<std::size_t>();
      }
      if (data.contains("non_zero_elements")) {
         jac.non_zero_elements = data["non_zero_elements"].get<std::size_t>();
      }
      if (data.contains("edges_in_dag")) {
         jac.edges_in_dag = data["edges_in_dag"].get<std::size_t>();
      }
      if (data.contains("tangent_cost")) {
         jac.tangent_cost = data["tangent_cost"].get<std::size_t>();
      } else if (data.contains("tangentCost")) {
         jac.tangent_cost = data["tangentCost"].get<std::size_t>();
      }
      if (data.contains("adjoint_cost")) {
         jac.adjoint_cost = data["adjoint_cost"].get<std::size_t>();
      } else if (data.contains("adjointCost")) {
         jac.adjoint_cost = data["adjointCost"].get<std::size_t>();
      }
      if (data.contains("is_accumulated")) {
         jac.is_accumulated = data["is_accumulated"].get<bool>();
      } else if (data.contains("jacobianAccumulated")) {
         jac.is_accumulated = data["jacobianAccumulated"].get<bool>();
      }

      chain.elemental_jacobians.push_back(jac);
   }

   return chain;
}

/**
 * @brief Serializes a JacobianChain into a JSON string in the JSON Graph
 * Format.
 *
 * @param chain The JacobianChain to serialize.
 * @return std::string The JSON string.
 */
inline auto jacobian_chain_to_json(const JacobianChain& chain) -> std::string {
   using json = nlohmann::json;
   json j;
   json graph;
   json nodes = json::array();
   json edges = json::array();

   std::map<std::size_t, std::size_t> node_sizes;

   for (const auto& jac : chain.elemental_jacobians) {
      json edge;
      edge["source"] = std::to_string(jac.i);
      edge["target"] = std::to_string(jac.j);

      json metadata;
      metadata["n"] = jac.n;
      metadata["m"] = jac.m;
      metadata["ku"] = jac.ku;
      metadata["kl"] = jac.kl;
      metadata["non_zero_elements"] = jac.non_zero_elements;
      metadata["edges_in_dag"] = jac.edges_in_dag;
      metadata["tangentCost"] = jac.tangent_cost;
      metadata["adjointCost"] = jac.adjoint_cost;
      metadata["jacobianAccumulated"] = jac.is_accumulated;

      edge["metadata"] = metadata;
      edges.push_back(edge);

      node_sizes[jac.i] = jac.n;
      node_sizes[jac.j] = jac.m;
   }

   for (const auto& [id, size] : node_sizes) {
      json node;
      node["id"] = std::to_string(id);
      node["metadata"] = {{"vectorSize", size}};
      nodes.push_back(node);
   }

   graph["nodes"] = nodes;
   graph["edges"] = edges;
   j["graph"] = graph;

   return j.dump();
}

/**
 * @brief Writes a JacobianChain to a JSON file.
 *
 * @param chain The JacobianChain to serialize.
 * @param filepath The path to the output file.
 */
inline void write_json(
     const JacobianChain& chain, const std::string& filepath) {
   std::ofstream file(filepath);
   file << jacobian_chain_to_json(chain);
   file.close();
}

/**
 * @brief Serializes a Sequence into a JSON string.
 *
 * @param seq The sequence to serialize.
 * @return std::string The JSON string.
 */
inline auto sequence_to_json(const Sequence& seq) -> std::string {
   using json = nlohmann::json;
   json j = json::array();

   for (const Operation& op : seq) {
      json step;

      std::vector<std::string> indices;
      if (op.action == Action::ACCUMULATION) {
         step["kind"] = "accumulate-edge";
         if (op.mode == Mode::TANGENT) {
            step["method"] = "acc-tan";
         } else {
            step["method"] = "acc-adj";
         }
         indices.push_back(std::to_string(op.i));
         indices.push_back(std::to_string(op.j + 1));
         step["fillIn"] = 0;
      } else {
         step["kind"] = "face";
         if (op.action == Action::MULTIPLICATION) {
            step["method"] = "elim-mul";
         } else if (op.mode == Mode::TANGENT) {
            step["method"] = "elim-tan";
         } else {
            step["method"] = "elim-adj";
         }
         indices.push_back(std::to_string(op.i));
         indices.push_back(std::to_string(op.k + 1));
         indices.push_back(std::to_string(op.j + 1));
         step["fillIn"] = -1;
      }
      step["indices"] = indices;
      step["totalCost"] = op.fma;
      step["threadID"] = op.thread;
      step["startTime"] = op.start_time;
      j.push_back(step);
   }
   return j.dump();
}

/**
 * @brief Writes a Sequence to a JSON file.
 *
 * @param seq The Sequence to serialize.
 * @param filepath The path to the output file.
 */
inline void write_json(const Sequence& seq, const std::string& filepath) {
   std::ofstream file(filepath);
   file << sequence_to_json(seq);
   file.close();
}

}  // namespace jcdp::util

#endif  // JCDP_UTIL_JSON_HPP_
