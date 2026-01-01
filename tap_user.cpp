
#include "core/object/class_db.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_dictionary.h"
#include "core/math/math_defs.h"
#include "core/math/audio_frame.h"
#include "core/math/vector2.h"

#include "tap_user.h"
#include "tap_circuit_types.h"

void TapUser::_bind_methods() {
  //register all methods not including those named *_internal
  ClassDB::bind_method(D_METHOD("get_state_missing"), &TapUser::get_state_missing);
  ClassDB::bind_method(D_METHOD("get_event_count"), &TapUser::get_event_count);
  ClassDB::bind_method(D_METHOD("pop_event"), &TapUser::pop_event);
  ClassDB::bind_method(D_METHOD("get_sample_count"), &TapUser::get_sample_count);

  ClassDB::bind_method(D_METHOD("add_pin", "label", "initial_state"), &TapUser::add_pin);
  ClassDB::bind_method(D_METHOD("add_pin_with_frame", "label", "initial_state"), &TapUser::add_pin_with_frame);
  ClassDB::bind_method(D_METHOD("remove_pin", "label"), &TapUser::remove_pin);

  ClassDB::bind_method(D_METHOD("get_pin_state", "label"), &TapUser::get_pin_state);
  ClassDB::bind_method(D_METHOD("all_pin_states"), &TapUser::all_pin_states);

  ClassDB::bind_method(D_METHOD("set_pin_state", "label", "new_state"), &TapUser::set_pin_state);
  ClassDB::bind_method(D_METHOD("set_pin_state_with_frame", "label", "new_state"), &TapUser::set_pin_state_with_frame);

  ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "state_missing"), "", "get_state_missing");
}

int TapUser::get_event_count() {
  return queue.get_population();
}

Vector2i TapUser::pop_event() {
  auto o_event = pop_event_internal();
  if (o_event.has_value() && pins.find(o_event->pid) != pins.end()) {
    tap_event_t event = o_event.value();
    tap_frame state = pins.get(event.pid).last_event.state;
    return Vector2i(state.left, state.right);
  }
  
  return STATE_MISSING;
}

std::optional<tap_event_t> TapUser::pop_event_internal() {
  if (queue.is_empty()) {
    return std::nullopt;
  }
  return queue.pop_minimum().first;
}

void TapUser::push_event_internal(tap_event_t event) {
  queue.insert(event, event.time);
}

int TapUser::get_sample_count() {
  return samples;
}

void TapUser::set_sample_count_internal(int new_samples) {
  samples = new_samples;
}

void TapUser::add_pin(tap_label_t label, Vector2i initial_state) {
  tap_frame frame(initial_state);
  
  pins.insert(label, tap_pin_t{Vector<tap_label_t>(), {0, frame}});
}

void TapUser::add_pin_with_frame(tap_label_t label, Vector2 initial_state) {
  tap_frame frame(AudioFrame(initial_state.x, initial_state.y));
  
  pins.insert(label, tap_pin_t{Vector<tap_label_t>(), {0, frame}});
}

void TapUser::remove_pin(tap_label_t label) {
  pins.erase(label);
}

Vector2i TapUser::get_pin_state(tap_label_t label) {
  if (pins.find(label) == pins.end()) {
    print_error("Attempted to get state of nonexistant pin " + itos(label));
    return STATE_MISSING;
  }

  tap_frame state = pins.get(label).last_event.state;
  return Vector2i(state.left, state.right);
}

TypedDictionary<tap_label_t, Vector2i> TapUser::all_pin_states() {
  TypedDictionary<tap_label_t, Vector2i> dict;
  for (const auto &pair : pins) {
    tap_label_t label = pair.key;
    tap_frame state = pair.value.last_event.state;
    dict[label] = Vector2i(state.left, state.right);
  }
  return dict;
}

void TapUser::set_pin_state(tap_label_t label, Vector2i new_state) {
  if (pins.find(label) == pins.end()) {
    print_error("Attempted to set state of nonexistant pin " + itos(label));
    return;
  }

  tap_frame frame(new_state);
  pins.get(label).last_event.state = frame;
}

void TapUser::set_pin_state_with_frame(tap_label_t label, Vector2 new_state) {
  if (pins.find(label) == pins.end()) {
    print_error("Attempted to set state of nonexistant pin " + itos(label));
    return;
  }

  tap_frame frame(AudioFrame(new_state.x, new_state.y));
  pins.get(label).last_event.state = frame;
}