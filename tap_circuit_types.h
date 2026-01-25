#pragma once

#include <cmath>
#include <cfloat>
#include <optional>

#include "core/object/object.h"
#include "core/object/ref_counted.h"

#include "core/math/vector2i.h"
#include "core/math/audio_frame.h"

#include "circuit.h"

/*
Convert tapped audio channel signals to integer channels.

The goal was to produce binary signals from the audio, but it's a little too
unstable to do that directly. The reason probably has to do with innaccuracies
in godot's audio playback. Might be good later to use wav files and a stream
generator for finer control.
*/
struct tap_frame {
  using bytes_t = uint16_t;

  union {
    struct {
      bytes_t left;
      bytes_t right;
    };
  };

  static constexpr float FLOAT_SCALE = (float)(1 << 15);
  static constexpr float FLOAT_SCALE_INV = 1.f / FLOAT_SCALE;

  //then, shift from 2^15 (32768 to -32768) to uint16_t range by adding an offset
  static constexpr float FLOAT_ADD = FLOAT_SCALE - 1.f; //2^15 - 1 in float

  //32-bit floating point should be accurate enough to retain all detail during
  //  this process
  static inline constexpr bytes_t channel_to_bytes(float channel) {
    if (channel >= 1.f) {
      return 0xFFFF;
    }
    if (channel <= -1.f) {
      return 0x0;
    }

    channel *= FLOAT_SCALE; //raise the range to be from -32768 to 32767
    channel += FLOAT_ADD; //shift to be from 0 to 65535
    return (bytes_t)(channel);
  }

  static inline constexpr float bytes_to_channel(bytes_t bytes) {
    // Map 0 to 65535 back to -1.0 to 1.0
    if (bytes >= 0xFFFF) {
      return 1.f;
    }
    if (bytes <= 0x0) {
      return -1.f;
    }

    float channel = (float)(bytes);
    channel -= FLOAT_ADD;
    channel *= FLOAT_SCALE_INV;
    return channel;
  }

  static inline constexpr bytes_t bytes_diff(tap_frame::bytes_t a, tap_frame::bytes_t b) {
    return a > b ? a - b : b - a;
  }

  inline constexpr tap_frame(AudioFrame frame) :
    left(channel_to_bytes(frame.left)), 
    right(channel_to_bytes(frame.right))
  {

  }

  inline constexpr tap_frame(Vector2i frame) :
    left(static_cast<bytes_t>(frame.x)),
    right(static_cast<bytes_t>(frame.y))
  {

  }

  inline constexpr tap_frame() :
    left(0),
    right(0)
  {

  }

  inline constexpr tap_frame(bytes_t l, bytes_t r) :
    left(l),
    right(r)
  {

  }

  inline constexpr AudioFrame audio_frame() const {
    return AudioFrame(bytes_to_channel(left), bytes_to_channel(left));
  }

  inline constexpr bytes_t delta(tap_frame with) const {
    return bytes_diff(left, with.left) + bytes_diff(right, with.right);
  }
};

/**
 * @brief Expose tap_frame functions to the editor for testing.
 */
class TapFrame : public RefCounted {
  GDCLASS(TapFrame, RefCounted)

  protected:
    static void _bind_methods() {
      ClassDB::bind_static_method("TapFrame", D_METHOD("to_bytes", "frame"), &TapFrame::to_bytes);
      ClassDB::bind_static_method("TapFrame", D_METHOD("to_channel", "bytes"), &TapFrame::to_channel);
    }
  
  public:
    static Vector2i to_bytes(Vector2 frame) {
      tap_frame b(AudioFrame(frame.x, frame.y));
      return Vector2i(b.left, b.right);
    }
    static Vector2 to_channel(Vector2i bytes) {
      tap_frame b(bytes.x, bytes.y);
      return b.audio_frame();
    }
};


//base tap types
typedef unsigned int tap_label_t; //used to identify components and pins in separate collections
typedef unsigned int tap_time_t; //sample count time
typedef uint16_t tap_state_t; //16-bit audio signal

//event tap types
typedef circuit_event_t<tap_frame, tap_time_t, tap_label_t, tap_label_t> tap_event_t;
typedef circuit_queue_t<tap_frame, tap_time_t, tap_label_t, tap_label_t> tap_queue_t;

//component tap types
typedef circuit_pin_t<tap_frame, tap_time_t, tap_label_t> tap_pin_t;
typedef circuit_component_type_t<tap_time_t, tap_label_t, const tap_event_t *, tap_queue_t> tap_component_type_t;
typedef circuit_component_t<tap_frame, tap_time_t, tap_label_t, tap_label_t, const tap_event_t *, tap_queue_t> tap_component_t;