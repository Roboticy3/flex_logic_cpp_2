
#include "core/object/class_db.h"

#include "tap_component_type.h"

// Define the static solver registry
HashMap<StringName, tap_solver_t> TapComponentType::solver_registry;

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

	for (auto const &[key, value] : solver_registry) {
		if (!first) {
			hint += ",";
		}
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
	component_type.solver = (void*)solver_registry.get(solver_name);
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

void wire_solver(tap_label_t cid, const tap_component_t &component, const Vector<const tap_event_t *> &state, tap_queue_t &queue, tap_time_t current_time) {
	//find the most recent activation
	tap_event_t latest;
	latest.time = (tap_time_t)(-1); //initialize to max value
	for (int i = 0; i < state.size(); i++) {
		//we're not going to beat current_time because the solver runs on an event
		//at current_time; any earlier event is thus "corrupted" or uninitialized,
		//something which will be fixed by the current event.
		if (state[i]->time == current_time) {
			latest = *state[i];
			break;
		}

		if (state[i]->time < latest.time && state[i]->time > current_time) {
			latest = *state[i];
		}
	}

	if (latest.time == (tap_time_t)(-1)) {
		return; //no activations found
	}

	//propogate to all other pins
	for (int i = 0; i < state.size(); i++) {
		if (state[i]->pid == latest.pid) {
			continue; //skip the source pin
		}

		tap_time_t new_time = latest.time + 1;

		//push to queue with a dummy time and pin ID
		queue.insert({ new_time, latest.state, state[i]->pid, cid }, new_time);
	}
}

void none_solver(tap_label_t cid, const tap_component_t &component, const Vector<const tap_event_t *> &state, tap_queue_t &queue, tap_time_t current_time) {
	// ¯\_(ツ)_/¯
}

void mixer_solver(tap_label_t cid, const tap_component_t &component, const Vector<const tap_event_t *> &state, tap_queue_t &queue, tap_time_t current_time) {
	//add in float space to act more like a mixxer than a binary adder
	tap_frame_t frame0 = state[0]->state;
	tap_frame_t frame1 = state[1]->state;

	tap_frame_t result = frame0 + frame1;
	tap_frame_t carry(0.0f, 0.0f);

	if (result.left < -1.0f) {
		carry.left = -1.0f;
		result.left = -1.0f;
	} else if (result.left > 1.0f) {
		carry.left = 1.0f;
		result.left = 1.0f;
	}

	if (result.right < -1.0f) {
		carry.right = -1.0f;
		result.right = -1.0f;
	} else if (result.right > 1.0f) {
		carry.right = 1.0f;
		result.right = 1.0f;
	}

	//print_line("mixed result ", Vector2(result.l, result.r), " from ", Vector2(frame0.l, frame0.r), " and ", Vector2(frame1.l, frame1.r), " with carry ", carry.left, ", ", carry.right);

	tap_time_t new_time = current_time + 2;

	// Push result to queue with a dummy time and pin ID
	queue.insert({ new_time, result, state[2]->pid, cid }, new_time);
	queue.insert({ new_time, carry, state[3]->pid, cid }, new_time);
}

void gate_solver(tap_label_t cid, const tap_component_t &component, const Vector<const tap_event_t *> &state, tap_queue_t &queue, tap_time_t current_time) {
	//multiply two inputs

	tap_frame_t frame0 = state[0]->state;
	tap_frame_t frame1 = state[1]->state;

	tap_frame_t result = frame0 * frame1;

	tap_time_t new_time = current_time + 3;

	queue.insert({ new_time, result, state[2]->pid, cid }, new_time);
}

void inverter_solver(tap_label_t cid, const tap_component_t &component, const Vector<const tap_event_t *> &state, tap_queue_t &queue, tap_time_t current_time) {
	//invert the input

	tap_frame_t frame = state[0]->state;

	tap_frame_t result = frame * -1;

	tap_time_t new_time = current_time + 3;

	queue.insert({ new_time, result, state[1]->pid, cid }, new_time);
}

void TapComponentType::initialize_solver_registry_internal() {
	solver_registry.clear();
	solver_registry.insert("wire", wire_solver);
	solver_registry.insert("none", none_solver);
	solver_registry.insert("mixer", mixer_solver);
	solver_registry.insert("gate", gate_solver);
	solver_registry.insert("inverter", inverter_solver);
	print_line(vformat("TapComponentType: Registered %d solver functions.", TapComponentType::solver_registry.size()));
}

void TapComponentType::uninitialize_solver_registry_internal() {
	solver_registry.clear();
}