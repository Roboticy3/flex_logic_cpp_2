#include <mutex>
#include <optional>

#include "core/object/class_db.h"

#include "core/object/object.h"
#include "tap_circuit_types.h"
#include "tap_component_type.h"
#include "tap_sim.h"

void TapSim::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_network"), &TapSim::get_network);
	ClassDB::bind_method(D_METHOD("set_network", "new_network"), &TapSim::set_network);

	ClassDB::bind_method(D_METHOD("get_patch_bay"), &TapSim::get_patch_bay);
	ClassDB::bind_method(D_METHOD("set_patch_bay", "new_patch_bay"), &TapSim::set_patch_bay);

	ClassDB::bind_method(D_METHOD("get_tick_rate"), &TapSim::get_tick_rate);
	ClassDB::bind_method(D_METHOD("set_tick_rate", "new_tick_rate"), &TapSim::set_tick_rate);

	ClassDB::bind_method(D_METHOD("get_latest_event_time"), &TapSim::get_latest_event_time);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "network", PROPERTY_HINT_RESOURCE_TYPE, "TapNetwork"), "set_network", "get_network");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "patch_bay", PROPERTY_HINT_RESOURCE_TYPE, "TapPatchBay"), "set_patch_bay", "get_patch_bay");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "tick_rate", PROPERTY_HINT_RANGE, "0,1024"), "set_tick_rate", "get_tick_rate");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "latest_event_time"), "", "get_latest_event_time");

	ClassDB::bind_method(D_METHOD("process_once"), &TapSim::process_once);
	ClassDB::bind_method(D_METHOD("process_to"), &TapSim::process_to);
	ClassDB::bind_method(D_METHOD("clear"), &TapSim::clear);
	ClassDB::bind_method(D_METHOD("instantiate"), &TapSim::instantiate);
}

Ref<TapNetwork> TapSim::get_network() const {
	return network;
}

void TapSim::set_network(Ref<TapNetwork> new_network) {
	network = new_network;
}

Ref<TapPatchBay> TapSim::get_patch_bay() const {
	return patch_bay;
}

void TapSim::set_patch_bay(Ref<TapPatchBay> new_patch_bay) {
	patch_bay = new_patch_bay;
}

int TapSim::get_tick_rate() const {
	return tick_rate;
}

void TapSim::set_tick_rate(int new_tick_rate) {
	tick_rate = new_tick_rate;
}

tap_time_t TapSim::get_latest_event_time() const {
	return latest_event_time;
}

void TapSim::process_once_internal(tap_queue_t &queue) {
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

void TapSim::process_once() {
	if (patch_bay.is_null()) {
		ERR_PRINT("TapSim::process_once: patch bay is not set. Cannot process events.");
		return;
	}

	tap_queue_t &queue = patch_bay->get_queue_internal();
	process_once_internal(queue);
}

int TapSim::process_to(tap_time_t end_time) {
	tap_queue_t &queue = patch_bay->get_queue_internal();
	int count = 0;
	while (!queue.is_empty() && queue.minimum().first.time <= end_time) {
		process_once_internal(queue);
		count++;
	}

	return count;
}

void TapSim::push_event(tap_time_t time, AudioFrame state, tap_label_t pid) {
	patch_bay->get_queue_internal().insert(tap_event_t{ time, state, pid, patch_bay->COMPONENT_MISSING }, time);
	latest_event_time = time > latest_event_time ? time : latest_event_time;
}

void TapSim::clear() {
	std::lock_guard<std::recursive_mutex> lock(mutex);
	patch_bay->clear_pins();
	network->clear_components();
}

std::recursive_mutex &TapSim::get_mutex() const {
	return mutex;
}

void TapSim::instantiate() {
	network.instantiate();
	patch_bay.instantiate();

	network->set_patch_bay(patch_bay);

	Ref<TapComponentType> wire;
	wire.instantiate();
	network->set_wire_type(wire);

	is_instantiated = true;
}

TapSim::TapSim() {
}