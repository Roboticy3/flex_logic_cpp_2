#include <optional>

#include "core/object/class_db.h"

#include "tap_circuit_types.h"
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

void TapSim::process_once() {
  /*
  tap_event_t next = patch_bay->pop_event_internal();

  //get the pin
  std::optional<tap_pin_t> pin = patch_bay->get_pin_internal(next.pid);
  tap_event_t *state = patch_bay->get_pin_state_internal(next.pid);

  if (pin.has_value()) {
    ERR_PRINT(String("Propogated event on bad pin id ") + itos(next.pid));
    return;
  }

  if (state == nullptr) {
    ERR_PRINT(String("State missing for pin ") + itos(next.pid));
    return;
  }

  //apply the new state

  //check previous event is processed (processing still possible, warn only)
  if (next.time < state.time) {
    WARN_PRINT(String("Space-time continuum broken by ") + itos(state.time - next.time) + String(" seconds on pin ") + itos(next.pid));
  }

  //propogate the event to the pin's connections
  for (tap_label_t component_label : pin->components) {
    tap_component_t *component = network->get_component_internal(component_label);
    
    //schizo-check. I think network interface covers this, but can't be too sure.
    if (component == nullptr) {
      WARN_PRINT(String("Pin ") + itos(next.pid) + String(" has connection to invalid component ") + itos(component_label));
      continue;
    }
  }
  */
}