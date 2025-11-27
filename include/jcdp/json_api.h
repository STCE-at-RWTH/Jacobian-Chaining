#ifndef JCDP_JSON_API_H
#define JCDP_JSON_API_H

#include <cstdint>

extern "C" {

/**
 * @brief Runs the JCDP solver based on a JSON configuration string.
 *
 * @param json_str          The JSON string describing the Jacobian chain.
 * @param optimizer         The optimizer to use ("dp" or "bnb").
 * @param scheduler         The scheduler to use ("list" or "bnb").
 * @param omp_threads       The number of OpenMP threads available.
 * @param available_threads The limit on the number of threads in the solution.
 * @param available_memory  The amount of memory available for the solution.
 * @param time_to_solve     The time limit for the solver in seconds.
 * @param result_buffer     Output parameter. Will be set to point to a
 *                          null-terminated string containing the result JSON.
 *                          The string is managed by the library and is valid
 *                          until the next call to this function.
 * @return 0 on success, non-zero on error.
 */
uint32_t jcdp_run_from_json(
     const char* json_str, const char* optimizer, const char* scheduler,
     uint32_t omp_threads, uint32_t available_threads,
     uint32_t available_memory, uint32_t time_to_solve, bool matrix_free,
     const char** result_buffer);
}

#endif  // JCDP_JSON_API_H
