#pragma once

#include "core/io/resource.h"
#include "core/templates/vector.h"
#include "core/math/audio_frame.h"
#include "core/string/string_name.h"

using ReferenceSimFunc = void(*)(Vector<AudioFrame> &solution,const Vector<AudioFrame> &problem);

class ReferenceSim : public Resource {
  GDCLASS(ReferenceSim, Resource)

  StringName reference_sim_name;
  ReferenceSimFunc reference_sim_func;

  protected:
    static void _bind_methods();
  
  public:
    StringName get_reference_sim_name() const;
    void set_reference_sim_name(const StringName &p_reference_sim_name);

    static void initialize_reference_registry_internal();
    static void deinitialize_reference_registry_internal();

    static HashMap<StringName, ReferenceSimFunc> reference_registry;
};
