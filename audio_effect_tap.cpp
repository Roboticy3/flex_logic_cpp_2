
#include "core/object/class_db.h"
#include "core/math/audio_frame.h"
#include "core/math/math_defs.h"

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
  instance->activation_delta = activation_delta;
  instance->component_id = component_id;
  return instance;
}

AudioEffectTap::AudioEffectTap() {
  Ref<CircuitTap> instance;
  instance.instantiate();
  circuit = instance;
}

void AudioEffectTapInstance::process(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) {

  for (int i = 0; i < p_frame_count; i++) {
    float l_activation = Math::abs(p_src_frames[i].l - last_activation.l);
    float r_activation = Math::abs(p_src_frames[i].r - last_activation.r);
    if (l_activation >= activation_delta || r_activation >= activation_delta) {
      circuit->queue.insert({component_id, p_src_frames[i], total_time + i}, total_time + i);
      last_activation = p_src_frames[i];
    }
    
  }

  // Pass through audio unchanged
  for (int i = 0; i < p_frame_count; i++) {
    p_dst_frames[i] = p_src_frames[i];
  }
}

bool AudioEffectTapInstance::process_silence() const {
  return true;
}