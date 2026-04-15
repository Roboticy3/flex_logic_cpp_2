#pragma once

#include "core/io/resource.h"

#include "tap_circuit_types.h"

void wire_solver(tap_label_t cid, const tap_component_t &component, const Vector<const tap_event_t *> &state, tap_queue_t &queue, tap_time_t current_time);

void none_solver(tap_label_t cid, const tap_component_t &component, const Vector<const tap_event_t *> &state, tap_queue_t &queue, tap_time_t current_time);

void mixer_solver(tap_label_t cid, const tap_component_t &component, const Vector<const tap_event_t *> &state, tap_queue_t &queue, tap_time_t current_time);

void gate_solver(tap_label_t cid, const tap_component_t &component, const Vector<const tap_event_t *> &state, tap_queue_t &queue, tap_time_t current_time);

void inverter_solver(tap_label_t cid, const tap_component_t &component, const Vector<const tap_event_t *> &state, tap_queue_t &queue, tap_time_t current_time);

/*
Define a resource wrapper for tap_component_type_t, allowing the user to
dynamically specify type names, sensitive pin indices, and solver functions via
a StringName. The solver stringname looks into a static registry of functions
belonging to this resource class.
*/
class TapComponentType : public Resource {
	GDCLASS(TapComponentType, Resource)

	tap_component_type_t component_type = {
		"Wire",
		Vector<int>(), //empty mask => all pins sensitive
		0, //pin count of 0 means variable
		(void*)wire_solver, //default to wire solver
	};
	StringName solver_function_name = "wire";
	size_t requested_memory_size = 0;

protected:
	static void _bind_methods();

public:
	void set_type_name(StringName new_name);
	StringName get_type_name();

	void set_sensitive_pins(const Vector<int> &new_sensitive_pins);
	Vector<int> get_sensitive_pins();

	void set_pin_count(int new_pin_count);
	int get_pin_count() const;

	void set_solver_function(StringName solver_name);
	StringName get_solver_function_name();

	void set_requested_memory_size(size_t new_size);
	size_t get_requested_memory_size() const;

	void set_component_type_internal(tap_component_type_t new_component_type);
	tap_component_type_t get_component_type_internal() const;

	static void initialize_solver_registry_internal();
	static void uninitialize_solver_registry_internal();

	static HashMap<StringName, tap_solver_t> solver_registry;

	TapComponentType() = default;
};