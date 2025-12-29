
#include "core/object/class_db.h"
#include "core/math/math_defs.h"
#include "core/math/audio_frame.h"
#include "core/math/vector2.h"

#include "tap_user.h"
#include "tap_circuit_types.h"

void TapUser::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_event_count"), &TapUser::get_event_count);
  ClassDB::bind_method(D_METHOD("pop_event"), &TapUser::pop_event);
  ClassDB::bind_method(D_METHOD("get_sample_count"), &TapUser::get_sample_count);
}

int TapUser::get_event_count() {
  return queue.get_population();
}

Vector2i TapUser::pop_event() {
  auto o_event = pop_event_internal();
  if (o_event.has_value()) {
    circuit_event_t<tap_frame, tap_time_t> event = o_event.value();
    return Vector2i(event.state.left, event.state.right);
  }
  
  return Vector2i(2, 2);
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