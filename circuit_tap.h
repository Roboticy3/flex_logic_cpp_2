#pragma once

#include <cmath>
#include <cfloat>
#include <optional>

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

  //go from 2^0 (1.0 to -1.0) to 2^15 by adding an exponent as an unsigned int.
  static constexpr uint32_t EXPONENT_ADD = 0x07800000; //15 in mantissa bits

  //then, shift from 2^15 (32768 to -32768) to uint16_t range by adding an offset
  static constexpr uint32_t FLOAT_ADD = 32768.0; //2^15 - 1 in float

  //32-bit floating point should be accurate enough to retain all detail during
  //  this process
  static inline bytes_t channel_to_bytes(float channel) {
    // Map -1.0 to 1.0 to 0 to 65535 with explicit quantization

    struct {
      union {
        float f;
        uint32_t u;
      };
    } converter;
    converter.f = channel;
    converter.u += EXPONENT_ADD;
    converter.f += FLOAT_ADD;
    return static_cast<bytes_t>(converter.f);
  }

  static inline float bytes_to_channel(bytes_t bytes) {
    // Map 0 to 65535 back to -1.0 to 1.0
    struct {
      union {
        float f;
        uint32_t b;
      };
    } converter;

    converter.f = static_cast<float>(bytes) - FLOAT_ADD;
    converter.b -= EXPONENT_ADD;
    return converter.f;
  }

  static inline constexpr bytes_t bytes_diff(tap_frame::bytes_t a, tap_frame::bytes_t b) {
    return a > b ? a - b : b - a;
  }

  inline tap_frame(AudioFrame frame) :
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

  protected:
    static void _bind_methods();

  public:

    //event queue
    tap_queue_t queue;

    //number of samples made on the last event pass
    int samples = 0;

    //components for processing
    HashMap<tap_label_t, tap_component_t> components;

    int get_event_count();
    Vector2i pop_event();
    std::optional<circuit_event_t<tap_frame, tap_time_t>> pop_event_internal();

    int get_sample_count();
    
    CircuitTap() = default;
};