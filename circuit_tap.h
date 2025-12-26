#pragma once

#include "core/object/class_db.h"
#include "core/io/resource.h"
#include "core/math/audio_frame.h"
#include "core/math/vector2.h"
#include "core/templates/hash_map.h"

#include "circuit.h"

typedef unsigned long long tap_label_t;
typedef unsigned long long tap_time_t;
typedef uint16_t tap_state_t;
typedef circuit_queue_t<AudioFrame, tap_time_t> tap_queue_t;
typedef circuit_component_t<AudioFrame, tap_time_t, tap_label_t> tap_component_t;

/*
Store templates for circuit primitives that can be attached to an audio tap
*/
class CircuitTap : public Resource {
  GDCLASS(CircuitTap, Resource)

  tap_queue_t queue;
  HashMap<tap_label_t, tap_component_t> components;

  protected:
    static void _bind_methods();

  public:

    Vector2 view_next_frame();
    
    tap_queue_t *qptrw();

    CircuitTap() = default;
};