
#include "core/object/class_db.h"

#include "reference_sim.h"

// Define the static registry
HashMap<StringName, ReferenceErrorFunc> ReferenceSim::reference_registry;

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

  ClassDB::bind_method(D_METHOD("stereo_error", "solution", "problem"), &ReferenceSim::stereo_error);

  ClassDB::bind_static_method("ReferenceSim", D_METHOD("initialize_reference_registry"), &ReferenceSim::initialize_reference_registry_internal);
  ClassDB::bind_static_method("ReferenceSim", D_METHOD("deinitialize_reference_registry"), &ReferenceSim::uninitialize_reference_registry_internal);
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

Vector2 ReferenceSim::stereo_error(PackedVector2Array solution, PackedVector2Array problem) const {
  Vector<AudioFrame> solution_vec;
  for (int i = 0; i < solution.size(); i++) {
    solution_vec.append(AudioFrame(solution[i].x, solution[i].y));
  }
  
  Vector<AudioFrame> problem_vec;
  for (int i = 0; i < problem.size(); i++) {
    problem_vec.append(AudioFrame(problem[i].x, problem[i].y));
  }
  
  AudioFrame error = stereo_error_internal(solution_vec, problem_vec);
  return Vector2(error.l, error.r);
}

AudioFrame ReferenceSim::stereo_error_internal(const Vector<AudioFrame> &solution, const Vector<AudioFrame> &problem) const {
  return reference_sim_func(solution, problem);
}

AudioFrame reference_mixer_no_peak(const Vector<AudioFrame> &solution, const Vector<AudioFrame> &problem) {

  if (solution.is_empty()) {
    return AudioFrame(Math::INF, Math::INF);
  }

  AudioFrame mix;
  for (AudioFrame p : problem) {
    mix = mix + p;
  }

  return mix - solution[0];
}

void ReferenceSim::initialize_reference_registry_internal() {
  reference_registry["mixer_no_peak"] = reference_mixer_no_peak;
  print_line(vformat("ReferenceSim: Registered %d reference functions.", reference_registry.size()));
}

void ReferenceSim::uninitialize_reference_registry_internal() {
  reference_registry.clear();
}



