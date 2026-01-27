#include <optional>

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
  ClassDB::bind_method(D_METHOD("push_event", "time", "state", "pid"), &TapPatchBay::push_event);
  ClassDB::bind_method(D_METHOD("pop_next_state"), &TapPatchBay::pop_next_state);

  ClassDB::bind_method(D_METHOD("get_next_state"), &TapPatchBay::get_next_state);
  ClassDB::bind_method(D_METHOD("get_next_pid"), &TapPatchBay::get_next_pid);
  ClassDB::bind_method(D_METHOD("get_next_time"), &TapPatchBay::get_next_time);

  ClassDB::bind_method(D_METHOD("add_pin", "initial_state"), &TapPatchBay::add_pin);
  ClassDB::bind_method(D_METHOD("has_pin", "label"), &TapPatchBay::has_pin);
  ClassDB::bind_method(D_METHOD("remove_pin", "label"), &TapPatchBay::remove_pin);

  ClassDB::bind_method(D_METHOD("get_pin_state", "label"), &TapPatchBay::get_pin_state);
  ClassDB::bind_method(D_METHOD("all_pin_states"), &TapPatchBay::all_pin_states);
  ClassDB::bind_method(D_METHOD("set_pin_state", "label", "new_state"), &TapPatchBay::set_pin_state);

  ClassDB::bind_method(D_METHOD("get_all_pin_connections"), &TapPatchBay::get_all_pin_connections);

  ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "state_missing"), "", "get_state_missing");
}

void TapPatchBay::push_event(tap_time_t time, Vector2 state, tap_state_t pid) {
  queue.insert({time, AudioFrame(state.x, state.y), pid, COMPONENT_MISSING}, time);
}

int TapPatchBay::get_event_count() const {
  return queue.get_population();
}

Vector2 TapPatchBay::pop_next_state() {
  if (queue.is_empty()) {
    return STATE_MISSING;
  }

  auto event = queue.pop_minimum().first;
  if (pins.label_get(event.pid).has_value()) {
    AudioFrame state = event.state;
    return Vector2(state.left, state.right);
  }
  
  return STATE_MISSING;
}

std::optional<tap_event_t> TapPatchBay::get_next_event_internal() {
  if (queue.is_empty()) {
    return std::nullopt;
  }

  auto pair = queue.minimum();
  if (pins.label_get(pair.first.pid).has_value()) {
    return pair.first;
  }
  
  return std::nullopt;
}

Vector2 TapPatchBay::get_next_state() {
  auto o_event = get_next_event_internal();
  if (o_event.has_value()) {
    AudioFrame state = o_event->state;
    return Vector2(state.left, state.right); 
  }

  return STATE_MISSING;
}

int TapPatchBay::get_next_pid() {
  auto o_event = get_next_event_internal();
  if (o_event.has_value()) {
    return o_event->pid;
  }

  return -1;
}

int TapPatchBay::get_next_time() {
  auto o_event = get_next_event_internal();
  if (o_event.has_value()) {
    return o_event->time;
  }

  return -1;
}



tap_queue_t &TapPatchBay::get_queue_internal() {
  return queue;
}

tap_label_t TapPatchBay::add_pin(Vector2 initial_state) {
  AudioFrame frame(initial_state.x, initial_state.y);

  tap_pin_t new_pin;
  new_pin.components = Vector<tap_label_t>(); // Initialize components
  tap_label_t result = pins.label_add(new_pin);

  if (result >= pin_states.size()) {
    pin_states.resize(result + 1);
  }

  pin_states.set(result, tap_event_t{0, frame, result, COMPONENT_MISSING});

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

static void attach_pin_single(tap_label_t label, Labeling<tap_pin_t> &pins, tap_label_t component_id) {
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

static void detach_pin_single(tap_label_t label, Labeling<tap_pin_t> &pins, tap_label_t component_id) {

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

TypedDictionary<tap_label_t, Vector2> TapPatchBay::all_pin_states() const {
  TypedDictionary<tap_label_t, Vector2> dict;
  for (int i = 0; i < pin_states.size(); i++) {
    dict[i] = Vector2(pin_states[i].state.left, pin_states[i].state.right);
  }
  return dict;
}

void TapPatchBay::set_pin_state(tap_label_t label, Vector2 new_state) {
  if (!pins.label_get_mut(label)) {
    print_error("Attempted to set state of nonexistant pin " + itos(label));
    return;
  }

  AudioFrame frame(new_state.x, new_state.y);
  get_state_internal(label)->state = frame;
}

Vector2 TapPatchBay::get_pin_state(tap_label_t label) const {
  if (!pins.label_get(label)) {
    print_error("Attempted to get state of nonexistant pin " + itos(label));
    return get_state_missing();
  }

  AudioFrame frame = pin_states[label].state;
  //print_line(itos(label), ": ", frame.left, ", ", frame.right);
  return Vector2(frame.left, frame.right);
}

std::optional<tap_pin_t> TapPatchBay::get_pin_internal(tap_label_t label) const {
  return pins.label_get(label);
}

tap_event_t *TapPatchBay::get_state_internal(tap_label_t label) {
  return &(pin_states.ptrw()[label]);
}

void TapPatchBay::clear_pins() {
  pins.clear();
  pin_states.clear();
}

TypedDictionary<tap_label_t, PackedInt64Array> TapPatchBay::get_all_pin_connections() const {
  TypedDictionary<tap_label_t, PackedInt64Array> dict;
  for (int i = 0; i < pins.size(); i++) {
    auto p_pin = pins.label_get(i);
    if (!p_pin.has_value()) {
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