#pragma once

#include "core/io/resource.h"
#include "core/templates/local_vector.h"
#include "core/math/audio_frame.h"
#include "core/string/string_name.h"

using ReferenceErrorFunc = AudioFrame(*)(const LocalVector<AudioFrame> &solution,const LocalVector<AudioFrame> &problem);

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
  AudioFrame total_error;
  
  ReferenceErrorFunc reference_sim_func;

  protected:
    static void _bind_methods();
  
  public:
    StringName get_reference_sim_name() const;
    void set_reference_sim_name(const StringName &new_reference_sim_name);

    Vector2 get_total_error() const;

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
     * AudioFrame error instead of Vector2.
     *
     * Intended for when `delta_time` gets very small in audio threads.
     */
    AudioFrame measure_error_internal(const LocalVector<AudioFrame> &solution, const LocalVector<AudioFrame> &problem, double delta_time);

    static HashMap<StringName, ReferenceErrorFunc> reference_registry;
};
