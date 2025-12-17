#ifndef JCDP_JSON_API_H
#define JCDP_JSON_API_H

#include <cstdint>

#include "jcdp/control.hpp"

extern "C" {

/**
 * @brief Initializes a new SolverState and returns its handle.
 *
 * @return int32_t The handle of the new state.
 */
int32_t jcdp_init();

/**
 * @brief Runs the JCDP solver based on a JSON configuration string.
 *
 * @param handle            The handle of the state to use. If 0, a new one is
 *                          created.
 * @param chain_json        The JSON string describing the Jacobian chain.
 * @param sequence_json     The JSON string describing a partial sequence.
 * @param optimizer         The optimizer to use ("dp" or "bnb").
 * @param scheduler         The scheduler to use ("list" or "bnb").
 * @param omp_threads       The number of OpenMP threads available.
 * @param available_threads The limit on the number of threads in the solution.
 * @param available_memory  The amount of memory available for the solution.
 * @param time_to_solve     The time limit for the solver in seconds.
 * @return 0 on success, or an error code (<0) on failure.
 */
int32_t jcdp_run_from_json(
     int32_t handle, const char* chain_json, const char* sequence_json,
     const char* optimizer, const char* scheduler, uint32_t omp_threads,
     uint32_t available_threads, uint32_t available_memory,
     uint32_t time_to_solve, bool matrix_free);

/**
 * @brief Pauses the execution associated with the given handle.
 *
 * @param handle The handle of the execution to pause.
 */
void jcdp_pause(int32_t handle);

/**
 * @brief Resumes the execution associated with the given handle.
 *
 * @param handle The handle of the execution to resume.
 */
void jcdp_resume(int32_t handle);

/**
 * @brief Cancels the execution associated with the given handle.
 *
 * @param handle The handle of the execution to cancel.
 */
void jcdp_cancel(int32_t handle);

/**
 * @brief Frees the result string associated with the given handle.
 *
 * @param handle The handle of the result to free.
 */
void jcdp_free_result(int32_t handle);
}

#endif  // JCDP_JSON_API_H
