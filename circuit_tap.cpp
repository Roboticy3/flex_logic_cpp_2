
#include "core/object/class_db.h"
#include "core/math/math_defs.h"
#include "core/math/audio_frame.h"
#include "core/math/vector2.h"

#include "circuit_tap.h"

void CircuitTap::_bind_methods() {
  ClassDB::bind_method(D_METHOD("pop_next_frame"), &CircuitTap::pop_next_frame);
  ClassDB::bind_method(D_METHOD("get_frame_count"), &CircuitTap::get_frame_count);
  ClassDB::bind_method(D_METHOD("get_last_frame_maximum"), &CircuitTap::get_last_frame_maximum);
  ClassDB::bind_method(D_METHOD("set_last_frame_maximum", "maximum"), &CircuitTap::set_last_frame_maximum);

  ADD_PROPERTY(PropertyInfo(Variant::INT, "last_frame_maximum", PROPERTY_HINT_NONE), "set_last_frame_maximum", "get_last_frame_maximum");
}

Vector2i CircuitTap::pop_next_frame() {
  if (queue.get_population() == 0) {
    return Vector2i(0, 0);
  }
  tap_frame f = queue.pop_minimum().first.state;
  return Vector2i(f.left, f.right);
}

int CircuitTap::get_frame_count() {
  return queue.get_population();
}

int CircuitTap::get_last_frame_maximum() {
  return last_frame_maximum;
}

void CircuitTap::set_last_frame_maximum(int p_maximum) {
  last_frame_maximum = p_maximum;
}