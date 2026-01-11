#include "core/object/class_db.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_dictionary.h"
#include "core/math/math_defs.h"
#include "core/math/audio_frame.h"
#include "core/math/vector2.h"

#include "tap_patch_bay.h"
#include "tap_circuit_types.h"

void TapPatchBay::_bind_methods() {
  //register all methods not including those named *_internal
  ClassDB::bind_method(D_METHOD("get_state_missing"), &TapPatchBay::get_state_missing);
  ClassDB::bind_method(D_METHOD("get_event_count"), &TapPatchBay::get_event_count);
  ClassDB::bind_method(D_METHOD("pop_event"), &TapPatchBay::pop_event);
  ClassDB::bind_method(D_METHOD("get_sample_count"), &TapPatchBay::get_sample_count);

  ClassDB::bind_method(D_METHOD("add_pin", "initial_state"), &TapPatchBay::add_pin);
  ClassDB::bind_method(D_METHOD("add_pin_with_frame", "initial_frame"), &TapPatchBay::add_pin_with_frame);
  ClassDB::bind_method(D_METHOD("has_pin", "label"), &TapPatchBay::has_pin);
  ClassDB::bind_method(D_METHOD("remove_pin", "label"), &TapPatchBay::remove_pin);

  ClassDB::bind_method(D_METHOD("get_pin_state", "label"), &TapPatchBay::get_pin_state);
  ClassDB::bind_method(D_METHOD("all_pin_states"), &TapPatchBay::all_pin_states);

  ClassDB::bind_method(D_METHOD("set_pin_state", "label", "new_state"), &TapPatchBay::set_pin_state);
  ClassDB::bind_method(D_METHOD("set_pin_state_with_frame", "label", "new_state"), &TapPatchBay::set_pin_state_with_frame);

  ClassDB::bind_method(D_METHOD("get_all_pin_connections"), &TapPatchBay::get_all_pin_connections);

  ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "state_missing"), "", "get_state_missing");
}

int TapPatchBay::get_event_count() const {
  return queue.get_population();
}

Vector2i TapPatchBay::pop_event() {
  if (queue.is_empty()) {
    return STATE_MISSING;
  }

  auto event = queue.pop_minimum().first;
  if (pins.label_get_mut(event.pid)) {
    tap_frame state = event.state;
    return Vector2i(state.left, state.right);
  }
  
  return STATE_MISSING;
}

tap_queue_t &TapPatchBay::get_queue_internal() {
  return queue;
}

int TapPatchBay::get_sample_count() const {
  return samples;
}

void TapPatchBay::set_sample_count_internal(int new_samples) {
  samples = new_samples;
}

tap_label_t TapPatchBay::add_pin(Vector2i initial_state) {
  tap_frame frame(initial_state);

  tap_pin_t new_pin;
  new_pin.components = Vector<tap_label_t>(); // Initialize components
  tap_label_t result = pins.label_add(new_pin);

  if (result >= pin_states.size()) {
    pin_states.resize(result + 1);
  }

  pin_states.set(result, tap_event_t{0, frame, result});

  return result;
}

tap_label_t TapPatchBay::add_pin_with_frame(Vector2 initial_frame) {
  tap_frame frame(AudioFrame(initial_frame.x, initial_frame.y));

  tap_pin_t new_pin;
  new_pin.components = Vector<tap_label_t>(); // Initialize components
  tap_label_t result = pins.label_add(new_pin);

  if (result >= pin_states.size()) {
    pin_states.resize(result + 1);
  }

  pin_states.set(result, tap_event_t{0, frame, result});

  return result;
}

bool TapPatchBay::has_pin(tap_label_t label) const {
  return pins.label_get(label).has_value();
}

bool TapPatchBay::remove_pin(tap_label_t label) {
  tap_label_t result = pins.label_remove(label);

  if (result) {
    pin_states.set(label, tap_event_t());
  }

  return result;
}

void attach_pin_single(tap_label_t label, Labeling<tap_pin_t> &pins, tap_label_t component_id) {
  auto p_pin = pins.label_get_mut(label);
  if (!p_pin) {
    WARN_PRINT("Attempted to attach nonexistant pin " + itos(label));
    return;
  }

  if (p_pin->components.find(component_id) == -1) {
    p_pin->components.push_back(component_id);
  } else {
    WARN_PRINT("Pin " + itos(label) + " already attached to component " + itos(component_id));
  }
}

void TapPatchBay::attach_pins_internal(const tap_component_t &component, tap_label_t component_id) {
  component.for_each_sensitive(attach_pin_single, pins, component_id);
}

void detach_pin_single(tap_label_t label, Labeling<tap_pin_t> &pins, tap_label_t component_id) {

  auto p_pin = pins.label_get_mut(label);
  if (!p_pin) {
    WARN_PRINT("Attempted to detach nonexistant pin " + itos(label));
    return;
  }

  int found = p_pin->components.find(component_id);
  if (found != -1) {
    p_pin->components.remove_at(found);
  } else {
    WARN_PRINT("Pin " + itos(label) + " not attached to component " + itos(component_id));
  }
}

void TapPatchBay::detach_pins_internal(const tap_component_t &component, tap_label_t component_label) {
  component.for_each_sensitive(detach_pin_single, pins, component_label);
}

Vector2i TapPatchBay::get_pin_state(tap_label_t label) const {
  if (pins[label].has_value() == false) {
    print_error("Attempted to get state of nonexistant pin " + itos(label));
    return STATE_MISSING;
  }

  tap_frame state = pin_states[label].state;
  return Vector2i(state.left, state.right);
}

TypedDictionary<tap_label_t, Vector2i> TapPatchBay::all_pin_states() const {
  TypedDictionary<tap_label_t, Vector2i> dict;
  for (tap_label_t i = 0; i < pins.size(); i++) {
    auto pin = pins[i];
    if (!pin.has_value()) {
      continue;
    }
    tap_frame state = pin_states[i].state;
    dict[i] = Vector2i(state.left, state.right);
  }
  return dict;
}

void TapPatchBay::set_pin_state(tap_label_t label, Vector2i new_state) {
  if (!pins.label_get_mut(label)) {
    print_error("Attempted to set state of nonexistant pin " + itos(label));
    return;
  }

  tap_frame frame(new_state);
  get_state_internal(label)->state = frame;
}

void TapPatchBay::set_pin_state_with_frame(tap_label_t label, Vector2 new_state) {
  if (!pins.label_get_mut(label)) {
    print_error("Attempted to set state of nonexistant pin " + itos(label));
    return;
  }

  tap_frame frame(AudioFrame(new_state.x, new_state.y));
  get_state_internal(label)->state = frame;
}

std::optional<tap_pin_t> TapPatchBay::get_pin_internal(tap_label_t label) const {
  return pins.label_get(label);
}

tap_event_t *TapPatchBay::get_state_internal(tap_label_t label) {
  return &(pin_states.ptrw()[label]);
}

TypedDictionary<tap_label_t, PackedInt64Array> TapPatchBay::get_all_pin_connections() const {
  TypedDictionary<tap_label_t, PackedInt64Array> dict;
  for (int i = 0; i < pins.size(); i++) {
    auto p_pin = pins[i];
    if (!p_pin) {
      continue;
    }

    PackedInt64Array p_connections;
    for (auto comp_id : p_pin->components) {
      p_connections.push_back(static_cast<int64_t>(comp_id));
    }

    dict[i] = p_connections;
  }
  return dict;
}