#pragma once

#include "core/object/class_db.h"
#include "core/io/resource.h"
#include "core/variant/variant.h"
#include "core/math/audio_frame.h"
#include "core/templates/hash_map.h"

#include "circuit_tap.h"
#include "circuit.h"

struct tap_frame {
  using bytes_t = uint16_t;

  union {
    struct {
      bytes_t left;
      bytes_t right;
    };
  };

  static inline constexpr bytes_t channel_to_bytes(float channel) {
    // Map -1.0 to 1.0 to 0 to 65535
    return static_cast<bytes_t>((channel + 1.0f) * 32767.5f);
  }

  static inline constexpr float bytes_to_channel(bytes_t bytes) {
    // Map 0 to 65535 back to -1.0 to 1.0
    return (static_cast<float>(bytes) / 32767.5f) - 1.0f;
  }

  static inline constexpr bytes_t bytes_diff(tap_frame::bytes_t a, tap_frame::bytes_t b) {
    return a > b ? a - b : b - a;
  }

  inline constexpr tap_frame(AudioFrame frame) :
    left(channel_to_bytes(frame.left)), 
    right(channel_to_bytes(frame.right))
  {

  }

  inline constexpr tap_frame() :
    left(0),
    right(0)
  {

  }

  inline constexpr AudioFrame audio_frame() const {
    return AudioFrame(bytes_to_channel(left), bytes_to_channel(left));
  }

  inline constexpr const bytes_t delta(tap_frame with) const {
    return bytes_diff(left, with.left) + bytes_diff(right, with.right);
  }
};

typedef unsigned int tap_label_t;
typedef unsigned int tap_time_t;
typedef uint16_t tap_state_t;
typedef circuit_queue_t<tap_frame, tap_time_t> tap_queue_t;
typedef circuit_component_t<tap_frame, tap_time_t, tap_label_t> tap_component_t;

/*
Store templates for circuit primitives that can be attached to an audio tap
*/
class CircuitTap : public Resource {
  GDCLASS(CircuitTap, Resource)

  //processing statistics to display for testing
  int last_frame_maximum = 0;
  tap_frame::bytes_t delta_avg = 0;
  tap_frame::bytes_t delta_max = 0;
  tap_frame::bytes_t delta_min = 0;

  protected:
    static void _bind_methods();

  public:

    tap_queue_t queue;
    HashMap<tap_label_t, tap_component_t> components;

    int get_frame_count();
    Vector2i pop_next_frame();

    int get_last_frame_maximum();
    void set_last_frame_maximum(int p_maximum);

    void flush_statistics();
    
    CircuitTap() = default;
};