
#include "core/object/class_db.h"
#include "core/math/audio_frame.h"

#include "audio_effect_tap.h"
#include "circuit_tap.h"

void AudioEffectTap::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_activation_delta"), &AudioEffectTap::get_activation_delta);
  ClassDB::bind_method(D_METHOD("set_activation_delta", "activation_delta"), &AudioEffectTap::set_activation_delta);

  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "frame_cache", PROPERTY_HINT_RESOURCE_TYPE, "FrameCache"), "set_frame_cache", "get_frame_cache");
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "activation_delta", PROPERTY_HINT_RANGE, "0.0,1.0"), "set_activation_delta", "get_activation_delta");
}

Ref<AudioEffectInstance> AudioEffectTap::instantiate() {
  Ref<AudioEffectTapInstance> instance;
  instance.instantiate();
  instance->circuit = circuit;
  return instance;
}

AudioEffectTap::AudioEffectTap() {
  Ref<CircuitTap> instance;
  instance.instantiate();
  circuit = instance;
}

void AudioEffectTapInstance::process(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) {
  
  // Pass through audio unchanged
  for (int i = 0; i < p_frame_count; i++) {
    p_dst_frames[i] = p_src_frames[i];
  }
}

bool AudioEffectTapInstance::process_silence() const {
  return true;
}