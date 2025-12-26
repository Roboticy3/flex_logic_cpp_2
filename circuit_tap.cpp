
#include "core/object/class_db.h"
#include "core/math/audio_frame.h"
#include "core/math/vector2.h"

#include "circuit_tap.h"

void CircuitTap::_bind_methods() {
  ClassDB::bind_method(D_METHOD("view_next_frame"), &CircuitTap::view_next_frame);
}

Vector2 CircuitTap::view_next_frame() {
  if (queue.get_population() == 0) {
    return Vector2(0.f, 0.f);
  }
  return Vector2(queue.minimum().first.state);
}