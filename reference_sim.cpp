
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


  ClassDB::bind_method(D_METHOD("get_total_error"), &ReferenceSim::get_total_error);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "total_error", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "", "get_total_error");

  ClassDB::bind_method(D_METHOD("measure_error", "solution", "problem"), &ReferenceSim::measure_error);
  ClassDB::bind_method(D_METHOD("reset"), &ReferenceSim::reset);

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

Vector2 ReferenceSim::get_total_error() const {
  return Vector2(total_error.l, total_error.r);
}

void ReferenceSim::reset() {
  total_error = AudioFrame(0, 0);
}

Vector2 ReferenceSim::measure_error(PackedVector2Array solution, PackedVector2Array problem) {
  LocalVector<AudioFrame> solution_vec;
  for (int i = 0; i < solution.size(); i++) {
    solution_vec.push_back(AudioFrame(solution[i].x, solution[i].y));
  }
  
  LocalVector<AudioFrame> problem_vec;
  for (int i = 0; i < problem.size(); i++) {
    problem_vec.push_back(AudioFrame(problem[i].x, problem[i].y));
  }
  
  AudioFrame error = measure_error_internal(solution_vec, problem_vec, 1.0f);
  return Vector2(error.l, error.r);
}

AudioFrame ReferenceSim::measure_error_internal(const LocalVector<AudioFrame> &solution, const LocalVector<AudioFrame> &problem, float delta_time) {
  AudioFrame error = reference_sim_func(solution, problem);
  error.l = error.l < 0 ? -error.l : error.l;
  error.r = error.r < 0 ? -error.r : error.r;
  total_error += AudioFrame(error.l * delta_time, error.r * delta_time);
  return error;
}

AudioFrame reference_mixer_no_peak(const LocalVector<AudioFrame> &solution, const LocalVector<AudioFrame> &problem) {

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



