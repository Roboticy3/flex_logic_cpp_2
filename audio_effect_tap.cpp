
#include "core/object/class_db.h"
#include "core/math/audio_frame.h"

#include "audio_effect_tap.h"

void AudioEffectTap::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_frame_cache"), &AudioEffectTap::get_frame_cache);
  ClassDB::bind_method(D_METHOD("set_frame_cache", "frame_cache"), &AudioEffectTap::set_frame_cache);

  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "frame_cache", PROPERTY_HINT_RESOURCE_TYPE, "FrameCache"), "set_frame_cache", "get_frame_cache");
}

Ref<AudioEffectInstance> AudioEffectTap::instantiate() {
  Ref<AudioEffectTapInstance> instance;
  instance.instantiate();
  instance->frame_cache = frame_cache;
  return instance;
}

Ref<FrameCache> AudioEffectTap::get_frame_cache() const {
  return frame_cache;
}

void AudioEffectTap::set_frame_cache(const Ref<FrameCache> &p_frame_cache) {
  frame_cache = p_frame_cache;
}

AudioEffectTap::AudioEffectTap() {
  Ref<FrameCache> instance;
  instance.instantiate();
  frame_cache = instance;
}

void AudioEffectTapInstance::process(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) {
  if (frame_cache.is_valid()) {
    frame_cache->ensure_size(p_frame_count);
    AudioFrame *buffer = frame_cache->get_frame_buffer_internal();
    memcpy(buffer, p_src_frames, sizeof(AudioFrame) * p_frame_count);
    frame_cache->increment_frame_id();
  }

  // Pass through audio unchanged
  for (int i = 0; i < p_frame_count; i++) {
    p_dst_frames[i] = p_src_frames[i];
  }
}

bool AudioEffectTapInstance::process_silence() const {
  return true;
}