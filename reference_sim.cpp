
#include "core/object/class_db.h"

#include "reference_sim.h"

// Define the static registry
HashMap<StringName, ReferenceSimFunc> ReferenceSim::reference_registry;

void ReferenceSim::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_reference_sim_name"), &ReferenceSim::get_reference_sim_name);
  ClassDB::bind_method(D_METHOD("set_reference_sim_name", "name"), &ReferenceSim::set_reference_sim_name);
  
  String hint;
	bool first = true;

	for (auto const &[key, value] : reference_registry) {
		if (!first) {
			hint += ",";
		}
		first = false;
		hint += String(key);
	}
  
  ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "reference_sim_name", PROPERTY_HINT_ENUM, hint), "set_reference_sim_name", "get_reference_sim_name");

  ClassDB::bind_static_method("ReferenceSim", D_METHOD("initialize_reference_registry"), &ReferenceSim::initialize_reference_registry_internal);
  ClassDB::bind_static_method("ReferenceSim", D_METHOD("deinitialize_reference_registry"), &ReferenceSim::deinitialize_reference_registry_internal);
}

StringName ReferenceSim::get_reference_sim_name() const {
  return reference_sim_name;
}

void ReferenceSim::set_reference_sim_name(const StringName& name) {
  if (!reference_registry.has(name)) {
    ERR_PRINT("Reference sim name '" + name + "' not found in registry");
    return;
  }

  reference_sim_name = name;
}

void reference_mixer_no_peak(Vector<AudioFrame> &solution, const Vector<AudioFrame> &problem) {
  solution.clear();

  AudioFrame mix;
  for (AudioFrame p : problem) {
    mix = mix + p;
  }

  solution.append(mix);
}

