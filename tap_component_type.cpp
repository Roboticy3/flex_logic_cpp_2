#include "core/object/class_db.h"

#include "tap_component_type.h"

// Define the static solver registry
HashMap<StringName, tap_component_type_t::solver_t> TapComponentType::solver_registry;

void TapComponentType::_bind_methods() {
  // Binding methods for Godot
  ClassDB::bind_method(D_METHOD("set_type_name", "new_name"), &TapComponentType::set_type_name);
  ClassDB::bind_method(D_METHOD("get_type_name"), &TapComponentType::get_type_name);

  ClassDB::bind_method(D_METHOD("set_sensitive_pins", "new_sensitive_pins"), &TapComponentType::set_sensitive_pins);
  ClassDB::bind_method(D_METHOD("get_sensitive_pins"), &TapComponentType::get_sensitive_pins);

  ClassDB::bind_method(D_METHOD("set_pin_count", "new_pin_count"), &TapComponentType::set_pin_count);
  ClassDB::bind_method(D_METHOD("get_pin_count"), &TapComponentType::get_pin_count);

  ClassDB::bind_method(D_METHOD("set_solver_function", "solver_name"), &TapComponentType::set_solver_function);
  ClassDB::bind_method(D_METHOD("get_solver_function_name"), &TapComponentType::get_solver_function_name);

  //build the possible values for solver_function enum hint
  String hint;
  bool first = true;

  for (auto const& [key, value] : solver_registry) {
    if (!first) hint += ", ";
    first = false;
    hint += String(key);
  }

  ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "type_name"), "set_type_name", "get_type_name");
  ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "sensitive_pins"), "set_sensitive_pins", "get_sensitive_pins");
  ADD_PROPERTY(PropertyInfo(Variant::INT, "pin_count"), "set_pin_count", "get_pin_count");
  ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "solver_function", PROPERTY_HINT_ENUM, hint), "set_solver_function", "get_solver_function_name");
}

void TapComponentType::set_type_name(StringName new_name) {
  component_type.name = new_name;
}

StringName TapComponentType::get_type_name() {
  return component_type.name;
}

void TapComponentType::set_sensitive_pins(const Vector<int> &new_sensitive_pins) {
  component_type.sensitive = new_sensitive_pins;
}

Vector<int> TapComponentType::get_sensitive_pins() {
  return component_type.sensitive;
}

void TapComponentType::set_pin_count(int new_pin_count) {
  component_type.pin_count = new_pin_count;
}

int TapComponentType::get_pin_count() const {
  return component_type.pin_count;
}

void TapComponentType::set_solver_function(StringName solver_name) {
  if (solver_registry.find(solver_name) == solver_registry.end()) {
    ERR_FAIL_MSG("Solver function not found in registry: " + String(solver_name));
  }
  solver_function_name = solver_name;
  component_type.solver = solver_registry.get(solver_name);
}

StringName TapComponentType::get_solver_function_name() {
  return solver_function_name;
}

void TapComponentType::set_component_type_internal(tap_component_type_t new_component_type) {
  component_type = new_component_type;
}

tap_component_type_t TapComponentType::get_component_type_internal() const {
  return component_type;
}

/*
Prebuilt solvers go here. 
*/

void wire_solver(const Vector<const tap_event_t *> &pins, tap_queue_t &queue, tap_time_t current_time) {
  //find the most recent activation
  tap_event_t latest;
  latest.time = (tap_time_t)(-1); //initialize to max value
  for (int i = 0; i < pins.size(); i++) {
    //we're not going to beat current_time because the solver runs on an event
    //at current_time; any earlier event is thus "corrupted" or uninitialized,
    //something which will be fixed by the current event. 
    if (pins[i]->time == current_time) {
      latest = *pins[i];
      break;
    }

    if (pins[i]->time < latest.time && pins[i]->time > current_time) {
      latest = *pins[i];
    }
  }

  if (latest.time == (tap_time_t)(-1)) {
    return; //no activations found
  }

  //propogate to all other pins
  for (int i = 0; i < pins.size(); i++) {
    if (pins[i]->pid == latest.pid) {
      continue; //skip the source pin
    }

    //push to queue with a dummy time and pin ID
    queue.insert({ latest.time + 1, latest.state, pins[i]->pid }, latest.time);
  }
}

void adder_solver(const Vector<const tap_event_t *> &pins, tap_queue_t &queue, tap_time_t current_time) {
  // Example adder solver implementation
  if (pins.size() < 4) {
    return; // Not enough inputs
  }

  uint32_t result_carry[2] = {
    (uint32_t)(pins[0]->state.left) + (uint32_t)(pins[1]->state.left),
    (uint32_t)(pins[0]->state.right) + (uint32_t)(pins[1]->state.right),
  };

  //any overflow from 32-bit addition is caught here
  uint32_t carry_mask = 0xFFFF0000;
  tap_frame carry = {
    tap_frame::bytes_t((result_carry[0] & carry_mask) ? 0xFFFF : 0x0000),
    tap_frame::bytes_t((result_carry[1] & carry_mask) ? 0xFFFF : 0x0000),
  }; 

  result_carry[0] &= ~carry_mask;
  result_carry[1] &= ~carry_mask;
  tap_frame result = {
    tap_frame::bytes_t(result_carry[0]),
    tap_frame::bytes_t(result_carry[1]),
  };

  tap_time_t new_time = current_time;

  // Push result to queue with a dummy time and pin ID
  queue.insert({new_time + 3, result, pins[2]->pid}, new_time);
  queue.insert({new_time + 3, carry, pins[3]->pid}, new_time);
}

void TapComponentType::initialize_solver_registry_internal() {
  TapComponentType::solver_registry.clear();
  TapComponentType::solver_registry.insert("adder", &adder_solver);
  TapComponentType::solver_registry.insert("wire", &wire_solver);
  print_line(vformat("TapComponentType: Registered %d solver functions.", TapComponentType::solver_registry.size()));
}

void TapComponentType::uninitialize_solver_registry_internal() {
  TapComponentType::solver_registry.clear();
}