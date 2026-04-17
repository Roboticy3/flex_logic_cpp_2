#pragma once

#include "core/object/ref_counted.h"
#include "servers/audio/audio_stream.h"
#include "tap_circuit_types.h"

/**
 * @brief A wrapper class for audio stream inputs to the TapCircuit simulator.
 * 
 * This class encapsulates an AudioStream mapped to a specific circuit pin ID,
 * with additional verification parameters. Designed to be editable in the Godot editor.
 */
class TapInput : public RefCounted {
  GDCLASS(TapInput, RefCounted);

private:
  Ref<AudioStream> stream;
  tap_label_t pid = -1;
  int verification_proportion = 0;

protected:
  static void _bind_methods();

public:
  TapInput();
  TapInput(Ref<AudioStream> p_stream, tap_label_t p_pid, int p_verification_proportion = 0);

  // Getters and setters
  Ref<AudioStream> get_stream() const;
  void set_stream(Ref<AudioStream> p_stream);

  tap_label_t get_pid() const;
  void set_pid(tap_label_t p_pid);

  int get_verification_proportion() const;
  void set_verification_proportion(int p_verification_proportion);
};
