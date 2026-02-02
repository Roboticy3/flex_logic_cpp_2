
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

void ReferenceSim::set_reference_sim_name(const StringName& new_reference_sim_name) {
  if (!reference_registry.has(new_reference_sim_name)) {
    ERR_PRINT("Reference sim name '" + new_reference_sim_name + "' not found in registry");
    return;
  }

  reference_sim_name = new_reference_sim_name;
  reference_sim_func = reference_registry[new_reference_sim_name];
}

void reference_mixer_no_peak(Vector<AudioFrame> &solution, const Vector<AudioFrame> &problem) {
  solution.clear();

  AudioFrame mix;
  for (AudioFrame p : problem) {
    mix = mix + p;
  }

  solution.append(mix);
}

void ReferenceSim::initialize_reference_registry_internal() {
  reference_registry["mixer_no_peak"] = reference_mixer_no_peak;
  print_line(vformat("ReferenceSim: Registered %d reference functions.", reference_registry.size()));
}

void ReferenceSim::deinitialize_reference_registry_internal() {
  reference_registry.clear();
}


