#pragma once

#include "core/io/resource.h"
#include "core/string/string_name.h"

#include "core/variant/variant.h"
#include "tap_circuit_types.h"
#include "tap_circuit.h"

//a reference function takes a solution and problem and returns an error,
//writing the correct solution to the solution vector in the process
using ReferenceErrorFunc = tap_frame_t(*)(LocalVector<tap_frame_t> &solution,const LocalVector<tap_frame_t> &problem);

/**
 * @brief Benchmark mode enumeration
 */
enum BenchmarkMode {
  BENCHMARK_MODE_PURE,
  BENCHMARK_MODE_TAP
};

/**
 * @brief Struct containing a benchmark function pointer and mode
 */
struct tap_benchmark_t {
  BenchmarkMode mode = BENCHMARK_MODE_PURE;
  ReferenceErrorFunc func = nullptr;
  Ref<TapCircuit> circuit;
  PackedInt64Array output_pids;
};

/***
 * @brief Wrapper for a reference function to validate TapCircuit behavior.
 * 
 * @param reference_sim_name The name of the reference function to use from
 * `ReferenceSim::reference_registry`.
 *
 * @param total_error The total error of the reference function.
 *
 * @param reference_sim_func The internal reference function object.
 */
class ReferenceSim : public Resource {
  GDCLASS(ReferenceSim, Resource)

  StringName reference_sim_name;
  tap_frame_t total_error;
  
  tap_benchmark_t bench;

  protected:
    static void _bind_methods();
  
  public:
    StringName get_reference_sim_name() const;
    void set_reference_sim_name(const StringName &new_reference_sim_name);

    Vector2 get_total_error() const;

    Ref<TapCircuit> get_circuit() const;

    /**
     * Reset `total_error` to 0.
     */
    void reset();

    /**
     * @brief Initialize the reference registry. Call in "register_types.cpp"
     */
    static void initialize_reference_registry_internal();

    /**
     * @brief Uninitialize the reference registry. Call in "unregister_types.cpp"
     */
    static void uninitialize_reference_registry_internal();

    /**
     * Compare the result of the reference function with the provided
     * solution/problem pair. Returns the error as a Vector2 and adds the error
     * to `total_error`.
     */
    Vector2 measure_error(PackedVector2Array solution, PackedVector2Array problem);
    
    /**
     * @brief Internal version of `measure_error` with added complexity of
     * taking a `delta_time` parameter to scale the error, and returning a raw
     * tap_frame_t error instead of Vector2.
     *
     * Intended for when `delta_time` gets very small in audio threads.
     */
    tap_frame_t measure_error_internal(LocalVector<tap_frame_t> &solution, const LocalVector<tap_frame_t> &problem, double delta_time);

    /**
     * @brief Returns true if the current reference function has mode BENCHMARK_MODE_TAP
     */
    bool needs_tap() const;

    static HashMap<StringName, tap_benchmark_t> reference_registry;
};
