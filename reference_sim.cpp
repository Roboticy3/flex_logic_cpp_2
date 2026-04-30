#include "core/math/math_defs.h"
#include "core/object/class_db.h"

#include "reference_sim.h"

// Define the static registry
HashMap<StringName, tap_benchmark_t> ReferenceSim::reference_registry;

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
  ClassDB::bind_method(D_METHOD("set_total_error", "total_error"), &ReferenceSim::set_total_error);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "total_error", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_total_error", "get_total_error");

  ClassDB::bind_method(D_METHOD("measure_error", "solution", "problem"), &ReferenceSim::measure_error);
  ClassDB::bind_method(D_METHOD("reset"), &ReferenceSim::reset);

  ClassDB::bind_method(D_METHOD("get_circuit"), &ReferenceSim::get_circuit);
  ClassDB::bind_method(D_METHOD("set_circuit", "circuit"), &ReferenceSim::set_circuit);
  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "circuit", PROPERTY_HINT_RESOURCE_TYPE, "TapCircuit"), "set_circuit", "get_circuit");
  
  ClassDB::bind_method(D_METHOD("get_output_pids"), &ReferenceSim::get_output_pids);
  ClassDB::bind_method(D_METHOD("set_output_pids", "output_pids"), &ReferenceSim::set_output_pids);
  ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT64_ARRAY, "output_pids", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "set_output_pids", "get_output_pids");

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
  bench = reference_registry[new_reference_sim_name];
  
  // Copy circuit and output_pids from registry to the new properties
  circuit = bench.circuit;
  output_pids = bench.output_pids;
}

Vector2 ReferenceSim::get_total_error() const {
  return Vector2(total_error.l, total_error.r);
}

void ReferenceSim::set_total_error(const Vector2& new_total_error) {
  total_error = tap_frame_t(new_total_error.x, new_total_error.y);
}

void ReferenceSim::reset() {
  //std::cout << "ReferenceSim::reset" << std::endl;
  total_error = tap_frame_t(0, 0);
  if (circuit.is_valid()) {
    circuit->reset_live_states();
  }
}

Vector2 ReferenceSim::measure_error(PackedVector2Array solution, PackedVector2Array problem) {
  LocalVector<tap_frame_t> solution_vec;
  for (int i = 0; i < solution.size(); i++) {
    solution_vec.push_back(tap_frame_t(solution[i].x, solution[i].y));
  }
  
  LocalVector<tap_frame_t> problem_vec;
  for (int i = 0; i < problem.size(); i++) {
    problem_vec.push_back(tap_frame_t(problem[i].x, problem[i].y));
  }
  
  tap_frame_t error = measure_error_internal(solution_vec, problem_vec, 1.0);
  return Vector2(error.l, error.r);
}

tap_frame_t ReferenceSim::measure_error_internal(LocalVector<tap_frame_t> &solution, const LocalVector<tap_frame_t> &problem, double delta_time) {
  tap_frame_t error;

  if (solution.size() == 0) {
    ERR_PRINT("ReferenceSim: Solution vector is empty");
    return tap_frame_t(Math::INF, Math::INF);
  }
  
  // Check mode before invoking function
  if (circuit.is_valid()) {
    //compare solution with circuit solution
    if (output_pids.size() == 0) {
      ERR_PRINT("ReferenceSim: No output pins specified for circuit mode");
      return tap_frame_t(Math::INF, Math::INF);
    }
    tap_frame_t total_solution;
    for (size_t i = 0; i < solution.size(); i++) {
      total_solution += solution[i];
    }
    tap_frame_t total_state;
    for (size_t i = 0; i < solution.size(); i++) {
      tap_frame_t pin_state = circuit->get_patch_bay()->get_pin_state_internal(output_pids[i]);
      total_state += pin_state;      
      solution[i] = pin_state;
    }
    error = total_solution - total_state;
  } else if (bench.func) {
    error = bench.func(solution, problem);
  } else {
    ERR_PRINT("ReferenceSim: No function or circuit specified");
    error = tap_frame_t(Math::INF, Math::INF);
  }

  if (isnan(error.l) || isnan(error.r)) {
    error = tap_frame_t(0.0, 0.0);
  }
  
  error.l = error.l < 0 ? -error.l : error.l;
  error.r = error.r < 0 ? -error.r : error.r;

  constexpr float EPSILON = 1e-5f;
  if (error.l < EPSILON) {
    error.l = 0.0f;
  }
  if (error.r < EPSILON) {
    error.r = 0.0f;
  }
  total_error += tap_frame_t((double)error.l * delta_time, (double)error.r * delta_time);
  return error;
}

static tap_frame_t reference_mixer_no_peak(LocalVector<tap_frame_t> &solution, const LocalVector<tap_frame_t> &problem) {

  if (solution.is_empty()) {
    return tap_frame_t(Math::INF, Math::INF);
  }

  if (problem.size() < 2) {
    return tap_frame_t(Math::INF, Math::INF);
  }

  tap_frame_t mix;
  for (tap_frame_t p : problem) {
    mix = mix + p;
  }
  tap_frame_t error = mix - solution[0];
  solution[0] = mix;

  for (size_t i = 1; i < solution.size(); i++) {
    error = error + solution[i];
    solution[i] = tap_frame_t();
  }

  //std::cout << "reference_mixer_no_peak: problem[0]=(" << problem[0].l << "," << problem[0].r << ") problem[1]=(" << problem[1].l << "," << problem[1].r << ") problem[2]=(" << problem[2].l << "," << problem[2].r << ") mix=(" << mix.l << "," << mix.r << ")" << std::endl;

  return error;
}

static tap_frame_t reference_identity(LocalVector<tap_frame_t> &solution, const LocalVector<tap_frame_t> &problem) {
  tap_frame_t error;
  for (size_t i = 0; i < MIN(solution.size(), problem.size()); i++) {
    error = error + (solution[i] - problem[i]);
    solution[i] = problem[i];
  }
  return error;
}

//inverter level
static tap_frame_t reference_1(LocalVector<tap_frame_t> &solution, const LocalVector<tap_frame_t> &problem) {
  tap_frame_t error = solution[0] - (problem[1] - problem[0] + problem[2]);
  return error;
}

/*
static Ref<TapCircuit> create_reference_circuit_test() {
  Ref<TapCircuit> circuit;
  circuit.instantiate();
  
  circuit->instantiate();
  
  auto pb = circuit->get_patch_bay();
  auto net = circuit->get_network();
  
  pb->add_pin(Vector2(0, 0));
  pb->add_pin(Vector2(0, 0));
  pb->add_pin(Vector2(0, 0));
  pb->add_pin(Vector2(0, 0));

  Ref<TapComponentType> mixer;
  
  mixer.instantiate();
  mixer->set_component_type_internal({
    "Mixer",
    {0, 1},
    4,
    (void*)mixer_solver,
  });

  net->set_component_types({
    mixer,
  });

  net->add_component({0, 1, 2, 3}, 0);
  
  return circuit;
}

static Ref<TapCircuit> create_reference_circuit_2() {
  Ref<TapCircuit> circuit;
  circuit.instantiate();

  circuit->instantiate();
  
  auto pb = circuit->get_patch_bay();
  auto net = circuit->get_network();

  //inputs
  pb->add_pin(Vector2(0, 0));
  pb->add_pin(Vector2(0, 0));
  pb->add_pin(Vector2(0, 0));

  //outputs
  pb->add_pin(Vector2(0, 0));

  //internal
  //gain out
  pb->add_pin(Vector2(0, 0));
  //gate out
  pb->add_pin(Vector2(0, 0));
  //mixer peak out
  pb->add_pin(Vector2(0, 0));

  //component types
  Ref<TapComponentType> mixer;
  Ref<TapComponentType> gain;
  Ref<TapComponentType> gate;

  mixer.instantiate();
  mixer->set_component_type_internal({
    "Mixer",
    {0, 1},
    4,
    (void*)mixer_solver,
  });

  gain.instantiate();
  gain->set_component_type_internal({
    "Gain",
    {0},
    2,
    (void*)TapGain::solver,
  });
  gain->set_requested_memory_size(128);

  gate.instantiate();
  gate->set_component_type_internal({
    "Gate",
    {0},
    2,
    (void*)gate_solver,
  });

  net->set_component_types({
    mixer,
    gain,
    gate
  });

  //add components
  //gain
  net->add_component({1, 4}, 1);
  //gate
  net->add_component({2, 4, 5}, 2);
  //mixer
  net->add_component({0, 5, 3, 6}, 0);

  return circuit;
}
  */

void ReferenceSim::initialize_reference_registry_internal() {
  //reference_registry["test_circuit"] = {BENCHMARK_MODE_PURE, nullptr, create_reference_circuit_test()};
  reference_registry["mixer_no_peak"] = {BENCHMARK_MODE_PURE, reference_mixer_no_peak};
  reference_registry["identity"] = {BENCHMARK_MODE_PURE, reference_identity};
  reference_registry["1"] = {BENCHMARK_MODE_PURE, reference_1};
  //reference_registry["2"] = {BENCHMARK_MODE_PURE, nullptr, create_reference_circuit_2()};
  print_line(vformat("ReferenceSim: Registered %d reference functions.", reference_registry.size()));
}

void ReferenceSim::uninitialize_reference_registry_internal() {
  reference_registry.clear();
}

Ref<TapCircuit> ReferenceSim::get_circuit() const {
  return circuit;
}

void ReferenceSim::set_circuit(Ref<TapCircuit> new_circuit) {
  circuit = new_circuit;
}

PackedInt64Array ReferenceSim::get_output_pids() const {
  return output_pids;
}

void ReferenceSim::set_output_pids(const PackedInt64Array &new_output_pids) {
  output_pids = new_output_pids;
}

bool ReferenceSim::needs_tap() const {
  return circuit.is_valid();
}
