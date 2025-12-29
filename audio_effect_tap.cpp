
#include "core/object/class_db.h"
#include "core/math/audio_frame.h"
#include "core/math/math_defs.h"

#include "audio_effect_tap.h"
#include "tap_circuit_types.h"
#include "tap_user.h"

void AudioEffectTap::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_activation_delta"), &AudioEffectTap::get_activation_delta);
  ClassDB::bind_method(D_METHOD("set_activation_delta", "new_activation_delta"), &AudioEffectTap::set_activation_delta);
  ClassDB::bind_method(D_METHOD("get_circuit"), &AudioEffectTap::get_circuit);
  ClassDB::bind_method(D_METHOD("set_circuit", "new_circuit"), &AudioEffectTap::set_circuit);

  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "circuit", PROPERTY_HINT_RESOURCE_TYPE, "TapUser"), "set_circuit", "get_circuit");
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "activation_delta", PROPERTY_HINT_RANGE, "0,65535"), "set_activation_delta", "get_activation_delta");
}

Ref<AudioEffectInstance> AudioEffectTap::instantiate() {
  Ref<AudioEffectTapInstance> instance;
  instance.instantiate();
  instance->circuit = circuit;
  instance->activation_delta = activation_delta;
  instance->pid = pid;
  return instance;
}

tap_frame::bytes_t AudioEffectTap::get_activation_delta() {
  return activation_delta;
}

void AudioEffectTap::set_activation_delta(tap_frame::bytes_t new_activation_delta) {
  activation_delta = new_activation_delta;
}

Ref<TapUser> AudioEffectTap::get_circuit() {
  return circuit;
}

void AudioEffectTap::set_circuit(Ref<TapUser> new_circuit) {
  circuit = new_circuit;

  if (circuit.is_null()) {
    circuit.instantiate();
  }
}

AudioEffectTap::AudioEffectTap() {}

void AudioEffectTapInstance::process(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) {

  //tap_frame::bytes_t max_delta = 0;
  //tap_frame::bytes_t max_value = 0;
  for (int i = 0; i < p_frame_count; i++) {
    tap_frame src_frame = tap_frame(p_src_frames[i]);
    //max_delta = last_activation.delta(src_frame) > max_delta ? last_activation.delta(src_frame):max_delta;
    //max_value = src_frame.left > max_value ? src_frame.left : max_value;
    //max_value = src_frame.right > max_value ? src_frame.right : max_value;
    if (last_activation.delta(src_frame) >= activation_delta) {
      circuit->push_event_internal({total_time + i, pid}, src_frame);
      last_activation = src_frame;
    }
  }

  circuit->set_sample_count_internal(p_frame_count);

  //print_line(vformat("Max delta for audio process step: %d", max_delta));
  //print_line(vformat("Max VALUE for audio process step: %d", max_value));

  // Pass through audio unchanged
  for (int i = 0; i < p_frame_count; i++) {
    p_dst_frames[i] = p_src_frames[i];
  }
}

bool AudioEffectTapInstance::process_silence() const {
  return true;
}