#pragma once

#include "core/io/resource.h"
#include "core/templates/local_vector.h"
#include "core/math/audio_frame.h"
#include "core/string/string_name.h"

using ReferenceErrorFunc = AudioFrame(*)(const LocalVector<AudioFrame> &solution,const LocalVector<AudioFrame> &problem);

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

    void reset();

    static void initialize_reference_registry_internal();
    static void uninitialize_reference_registry_internal();

    Vector2 measure_error(PackedVector2Array solution, PackedVector2Array problem);
    AudioFrame measure_error_internal(const LocalVector<AudioFrame> &solution, const LocalVector<AudioFrame> &problem, float delta_time);

    static HashMap<StringName, ReferenceErrorFunc> reference_registry;
};
