#include <mutex>
#include <optional>
#include <iostream>

#include "core/object/class_db.h"

#include "core/object/object.h"
#include "tap_circuit_types.h"
#include "tap_component_type.h"
#include "tap_circuit.h"

void TapCircuit::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_network"), &TapCircuit::get_network);
	ClassDB::bind_method(D_METHOD("set_network", "new_network"), &TapCircuit::set_network);

	ClassDB::bind_method(D_METHOD("get_patch_bay"), &TapCircuit::get_patch_bay);
	ClassDB::bind_method(D_METHOD("set_patch_bay", "new_patch_bay"), &TapCircuit::set_patch_bay);

	ClassDB::bind_method(D_METHOD("get_tick_rate"), &TapCircuit::get_tick_rate);
	ClassDB::bind_method(D_METHOD("set_tick_rate", "new_tick_rate"), &TapCircuit::set_tick_rate);

	ClassDB::bind_method(D_METHOD("get_latest_event_time"), &TapCircuit::get_latest_event_time);
	ClassDB::bind_method(D_METHOD("get_event_count"), &TapCircuit::get_event_count);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "network", PROPERTY_HINT_RESOURCE_TYPE, "TapNetwork"), "set_network", "get_network");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "patch_bay", PROPERTY_HINT_RESOURCE_TYPE, "TapPatchBay"), "set_patch_bay", "get_patch_bay");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "tick_rate", PROPERTY_HINT_RANGE, "0,1024"), "set_tick_rate", "get_tick_rate");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "latest_event_time"), "", "get_latest_event_time");

	ClassDB::bind_method(D_METHOD("process_once"), &TapCircuit::process_once);
	ClassDB::bind_method(D_METHOD("process_to"), &TapCircuit::process_to);
	ClassDB::bind_method(D_METHOD("clear"), &TapCircuit::clear);
	ClassDB::bind_method(D_METHOD("instantiate"), &TapCircuit::instantiate);

	ClassDB::bind_method(D_METHOD("are_any_pids_connected", "from", "to"), &TapCircuit::are_any_pids_connected);
}

Ref<TapNetwork> TapCircuit::get_network() const {
	std::lock_guard<std::recursive_mutex> lock(mutex);
	return network;
}

void TapCircuit::set_network(Ref<TapNetwork> new_network) {
	std::lock_guard<std::recursive_mutex> lock(mutex);
	network = new_network;
}

Ref<TapPatchBay> TapCircuit::get_patch_bay() const {
	std::lock_guard<std::recursive_mutex> lock(mutex);
	return patch_bay;
}

void TapCircuit::set_patch_bay(Ref<TapPatchBay> new_patch_bay) {
	std::lock_guard<std::recursive_mutex> lock(mutex);
	patch_bay = new_patch_bay;
}

int TapCircuit::get_tick_rate() const {
	std::lock_guard<std::recursive_mutex> lock(mutex);
	return tick_rate;
}

void TapCircuit::set_tick_rate(int new_tick_rate) {
	std::lock_guard<std::recursive_mutex> lock(mutex);
	tick_rate = new_tick_rate;
}

tap_time_t TapCircuit::get_latest_event_time() const {
	std::lock_guard<std::recursive_mutex> lock(mutex);
	return latest_event_time;
}

size_t TapCircuit::get_event_count() const {
	std::lock_guard<std::recursive_mutex> lock(mutex);
	return patch_bay->get_queue_internal().get_population();
}

bool TapCircuit::are_any_pids_connected(PackedInt64Array from, PackedInt64Array to) const {
	std::lock_guard<std::recursive_mutex> lock(mutex);

	if (from.size() == 0 || to.size() == 0) {
		return false;
	}

	HashSet<tap_label_t> end_pids;
	for (int i = 0; i < to.size(); i++) {
		end_pids.insert(to[i]);
	}

	for (tap_label_t start_pid : from) {
		//std::cout << "testing from: " << start_pid << std::endl;

		auto o_start = patch_bay->get_pin_internal(start_pid);
		if (!o_start.has_value()) {
			return false;
		}

		//directed dfs from start to end, treating pins as vertices and components as
		//edges
		LocalVector<tap_pin_t> stack_pins = { o_start.value() };
		HashSet<tap_label_t> visited_pids = { start_pid };
		HashSet<tap_label_t> visited_cids;

		while (!stack_pins.is_empty()) {

			size_t stack_end = stack_pins.size() - 1;
			const tap_pin_t pin = stack_pins[stack_end];
			
			stack_pins.resize(stack_end);
			
			for (tap_label_t cid : pin.components) {

				auto o_component = network->get_component_internal(cid);
				
				/* This case should be covered via the network interface.
				 * Likewise for pins.
				if (!o_component.has_value()) {
					continue;
				}
				*/

				if (visited_cids.has(cid)) {
					//std::cout << "\tskipping component: " << cid << std::endl;
					continue;
				}

				//std::cout << "\tvisiting component: " << cid << std::endl;
				visited_cids.insert(cid);

				const tap_component_t &component = o_component.value();
				for (tap_label_t pid2 : component.pins) {

					auto o_pin2 = patch_bay->get_pin_internal(pid2);
					/*
					if (!o_pin2.has_value()) {
						continue;
					}
					*/

					if (end_pids.has(pid2)) {
						//std::cout << "\t\tfound end pin: " << pid2 << std::endl;
						return true;
					}

					if (visited_pids.has(pid2)) {
						//std::cout << "\t\tskipping pin: " << pid2 << std::endl;
						continue;
					}

					//std::cout << "\t\tvisiting pin: " << pid2 << std::endl;
					visited_pids.insert(pid2);
					stack_pins.push_back(o_pin2.value());
				}
			}
		}
	}

	return false;
}

void TapCircuit::process_once_internal(tap_queue_t &queue) {
	if (queue.is_empty()) {
		ERR_PRINT(String("Tried to process empty queue"));
		return;
	}

	tap_event_t event = queue.pop_minimum().first;

	//get the pin
	std::optional<tap_pin_t> pin = patch_bay->get_pin_internal(event.pid);
	tap_event_t *state = patch_bay->get_state_internal(event.pid);

	//shouldn't need these checks later - assume the circuit is well-formed from outside
	if (!pin.has_value()) {
		ERR_PRINT(String("Propogated event on bad pin id ") + itos(event.pid));
		return;
	}

	if (state == nullptr) {
		ERR_PRINT(String("State missing for pin ") + itos(event.pid));
		return;
	}

	//apply the new state
	//note mutation happens here in the event handler, not in solvers themselves
	*state = event;

	//propogate the event to the pin's connections
	//the "sensitive" mechanic is handled in such a way that these components represent only the sensitive connections
	for (tap_label_t cid : pin->components) {
		if (cid == event.source_cid) {
			continue;
		}

		std::optional<tap_component_t> component = network->get_component_internal(cid);

		//since components cannot be modified outisde of the interface, this should never happen
		if (!component.has_value()) {
			ERR_PRINT(String("Propogated event on bad component id ") + itos(cid));
			continue;
		}

		//print_line("Solving component " + itos(cid) + " due to event on pin " + itos(event.pid));

		//get the input state for the component,
		Vector<const tap_event_t *> input;
		input.reserve(component->pins.size());
		for (tap_label_t pid : component->pins) {

			tap_event_t *component_state = patch_bay->get_state_internal(pid);
			input.push_back(component_state);
		}

		//solve the component
		component->component_type.solver(input, queue, event.time, cid);
	}
}

void TapCircuit::process_once() {
	if (patch_bay.is_null()) {
		ERR_PRINT("TapCircuit::process_once: patch bay is not set. Cannot process events.");
		return;
	}

	tap_queue_t &queue = patch_bay->get_queue_internal();
	process_once_internal(queue);
}

int TapCircuit::process_to(tap_time_t end_time) {
	tap_queue_t &queue = patch_bay->get_queue_internal();
	int count = 0;
	while (!queue.is_empty() && queue.minimum().first.time <= end_time) {
		process_once_internal(queue);
		count++;
	}

	return count;
}

void TapCircuit::push_event(tap_time_t time, AudioFrame state, tap_label_t pid) {
	patch_bay->get_queue_internal().insert(tap_event_t{ time, state, pid, patch_bay->COMPONENT_MISSING }, time);
	latest_event_time = time > latest_event_time ? time : latest_event_time;
}

void TapCircuit::clear() {
	std::lock_guard<std::recursive_mutex> lock(mutex);
	patch_bay->clear_pins();
	network->clear_components();
}

std::recursive_mutex &TapCircuit::get_mutex() const {
	return mutex;
}

void TapCircuit::instantiate() {
	network.instantiate();
	patch_bay.instantiate();

	network->set_patch_bay(patch_bay);

	Ref<TapComponentType> wire;
	wire.instantiate();
	network->set_wire_type(wire);

	instantiated = true;
}

bool TapCircuit::is_instantiated() const {
	return instantiated;
}

TapCircuit::TapCircuit() {
}