#pragma once

#include "core/io/resource.h"
#include "core/templates/vector.h"
#include "core/math/audio_frame.h"
#include "core/string/string_name.h"

using ReferenceErrorFunc = AudioFrame(*)(const Vector<AudioFrame> &solution,const Vector<AudioFrame> &problem);

class ReferenceSim : public Resource {
  GDCLASS(ReferenceSim, Resource)

  StringName reference_sim_name;
  ReferenceErrorFunc reference_sim_func;

  protected:
    static void _bind_methods();
  
  public:
    StringName get_reference_sim_name() const;
    void set_reference_sim_name(const StringName &new_reference_sim_name);

    static void initialize_reference_registry_internal();
    static void uninitialize_reference_registry_internal();

    Vector2 stereo_error(PackedVector2Array solution, PackedVector2Array problem) const;
    AudioFrame stereo_error_internal(const Vector<AudioFrame> &solution, const Vector<AudioFrame> &problem) const;

    static HashMap<StringName, ReferenceErrorFunc> reference_registry;
};
