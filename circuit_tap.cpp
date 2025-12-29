
#include "core/object/class_db.h"
#include "core/math/math_defs.h"
#include "core/math/audio_frame.h"
#include "core/math/vector2.h"

#include "circuit_tap.h"

void CircuitTap::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_event_count"), &CircuitTap::get_event_count);
  ClassDB::bind_method(D_METHOD("pop_event"), &CircuitTap::pop_event);
  ClassDB::bind_method(D_METHOD("get_sample_count"), &CircuitTap::get_sample_count);
}

int CircuitTap::get_event_count() {
  return queue.get_population();
}

Vector2i CircuitTap::pop_event() {
  auto o_event = pop_event_internal();
  if (o_event.has_value()) {
    circuit_event_t<tap_frame, tap_time_t> event = o_event.value();
    return Vector2i(event.state.left, event.state.right);
  }
  
  return Vector2i(2, 2);
}

std::optional<circuit_event_t<tap_frame, tap_time_t>> CircuitTap::pop_event_internal() {
  if (queue.is_empty()) {
    return std::nullopt;
  }
  return queue.pop_minimum().first;
}

int CircuitTap::get_sample_count() {
  return samples;
}