
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

  ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "state_missing"), "", "get_state_missing");
}

int TapPatchBay::get_event_count() {
  return queue.get_population();
}

Vector2i TapPatchBay::pop_event() {
  auto o_event = pop_event_internal();
  if (o_event.has_value() && pins.label_get(o_event->pid).has_value()) {
    tap_event_t event = o_event.value();
    tap_frame state = pin_states[o_event->pid].state;
    return Vector2i(state.left, state.right);
  }
  
  return STATE_MISSING;
}

std::optional<tap_event_t> TapPatchBay::pop_event_internal() {
  if (queue.is_empty()) {
    return std::nullopt;
  }
  return queue.pop_minimum().first;
}

void TapPatchBay::push_event_internal(tap_event_t event) {
  queue.insert(event, event.time);
}

int TapPatchBay::get_sample_count() {
  return samples;
}

void TapPatchBay::set_sample_count_internal(int new_samples) {
  samples = new_samples;
}

tap_label_t TapPatchBay::add_pin(Vector2i initial_state) {
  tap_frame frame(initial_state);

  tap_label_t result = pins.label_add(tap_pin_t());

  if (result >= pin_states.size()) {
    pin_states.resize(result + 1);
  }

  pin_states.set(result, tap_event_t{0, frame, result});

  return result;
}

tap_label_t TapPatchBay::add_pin_with_frame(Vector2 initial_frame) {
  tap_frame frame(AudioFrame(initial_frame.x, initial_frame.y));

  tap_label_t result = pins.label_add(tap_pin_t());

  if (result >= pin_states.size()) {
    pin_states.resize(result + 1);
  }

  pin_states.set(result, tap_event_t{0, frame, result});

  return result;
}

bool TapPatchBay::has_pin(tap_label_t label) {
  return pins.label_get(label).has_value();
}

bool TapPatchBay::remove_pin(tap_label_t label) {
  tap_label_t result = pins.label_remove(label);

  if (result) {
    pin_states.set(label, tap_event_t());
  }

  return result;
}

Vector2i TapPatchBay::get_pin_state(tap_label_t label) {
  if (pins[label].has_value() == false) {
    print_error("Attempted to get state of nonexistant pin " + itos(label));
    return STATE_MISSING;
  }

  tap_frame state = pin_states[label].state;
  return Vector2i(state.left, state.right);
}

TypedDictionary<tap_label_t, Vector2i> TapPatchBay::all_pin_states() {
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
  if (pins.label_get(label).has_value() == false) {
    print_error("Attempted to set state of nonexistant pin " + itos(label));
    return;
  }

  tap_frame frame(new_state);
  pin_states.ptrw()[label].state = frame;
}

void TapPatchBay::set_pin_state_with_frame(tap_label_t label, Vector2 new_state) {
  if (pins.label_get(label).has_value() == false) {
    print_error("Attempted to set state of nonexistant pin " + itos(label));
    return;
  }

  tap_frame frame(AudioFrame(new_state.x, new_state.y));
  pin_states.ptrw()[label].state = frame;
}