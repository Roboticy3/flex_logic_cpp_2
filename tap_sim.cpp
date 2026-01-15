#include <optional>

#include "core/object/class_db.h"

#include "tap_circuit_types.h"
#include "tap_component_type.h"
#include "tap_sim.h"

void TapSim::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_network"), &TapSim::get_network);
  ClassDB::bind_method(D_METHOD("set_network", "new_network"), &TapSim::set_network);
  
  ClassDB::bind_method(D_METHOD("get_patch_bay"), &TapSim::get_patch_bay);
  ClassDB::bind_method(D_METHOD("set_patch_bay", "new_patch_bay"), &TapSim::set_patch_bay);

  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "network", PROPERTY_HINT_RESOURCE_TYPE, "TapNetwork"), "set_network", "get_network");
  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "patch_bay", PROPERTY_HINT_RESOURCE_TYPE, "TapPatchBay"), "set_patch_bay", "get_patch_bay");

  ClassDB::bind_method(D_METHOD("process_once"), &TapSim::process_once);
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

void TapSim::process_once_internal(tap_queue_t &queue) {

  if (queue.is_empty()) {
    ERR_PRINT(String("Tried to process empty queue"));
    return;
  }
  
  tap_event_t next = queue.pop_minimum().first;

  //get the pin
  std::optional<tap_pin_t> pin = patch_bay->get_pin_internal(next.pid);
  tap_event_t *state = patch_bay->get_state_internal(next.pid);

  if (!pin.has_value()) {
    ERR_PRINT(String("Propogated event on bad pin id ") + itos(next.pid));
    return;
  }

  if (state == nullptr) {
    ERR_PRINT(String("State missing for pin ") + itos(next.pid));
    return;
  }

  //apply the new state
  //note mutation happens here in the event handler, not in solvers themselves
  *state = next;

  //propogate the event to the pin's connections
  //the "sensitive" mechanic is handled in such a way that these components represent only the sensitive connections
  for (tap_label_t cid : pin->components) {
    if (cid == next.source_cid) {
      continue;
    }

    std::optional<tap_component_t> component = network->get_component_internal(cid);

    //since components cannot be modified outisde of the interface, this should never happen
    if (!component.has_value()) {
      continue;
    }

    print_line("Solving component " + itos(cid) + " due to event on pin " + itos(next.pid));

    //get the input state for the component, 
    Vector<const tap_event_t *> input;
    input.reserve(component->pins.size());
    for (tap_label_t pin : component->pins) {
      input.push_back(patch_bay->get_state_internal(pin));
    }

    //solve the component
    component->component_type.solver(input, queue, next.time, cid);
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

TapSim::TapSim() {
  network.instantiate();
  patch_bay.instantiate();
  
  network->set_patch_bay(patch_bay);

  Ref<TapComponentType> wire;
  wire.instantiate();
  network->set_wire_type(wire);
}