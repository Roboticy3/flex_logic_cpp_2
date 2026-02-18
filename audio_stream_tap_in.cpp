#include "audio_stream_tap_in.h"
#include "core/variant/variant.h"
#include "scene/resources/audio_stream_wav.h"
#include "servers/audio/audio_stream.h"
#include "tap_circuit_types.h"

void AudioStreamTapIn::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_simulator"), &AudioStreamTapIn::get_simulator);
  ClassDB::bind_method(D_METHOD("set_simulator", "sim"), &AudioStreamTapIn::set_simulator);
  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "simulator", PROPERTY_HINT_RESOURCE_TYPE, "TapSim"), "set_simulator", "get_simulator");

  ClassDB::bind_method(D_METHOD("get_input_pids"), &AudioStreamTapIn::get_input_pids);
  ClassDB::bind_method(D_METHOD("set_input_pids", "pids"), &AudioStreamTapIn::set_input_pids);
  ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT64_ARRAY, "input_pids"), "set_input_pids", "get_input_pids");

  ClassDB::bind_method(D_METHOD("get_live"), &AudioStreamTapIn::get_live);
  ADD_PROPERTY(PropertyInfo(Variant::BOOL, "live", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "", "get_live");
}

Ref<AudioStreamPlayback> AudioStreamTapIn::instantiate_playback() {

  if (!base.is_valid()) {
    ERR_PRINT("Base audio stream is not valid");
    return Ref<AudioStreamPlayback>();
  }

  // Create a new instance of AudioStreamTapInPlayback
  Ref<AudioStreamTapInPlayback> playback;
  playback.instantiate();
  
  // Set the owner of the playback to this AudioStreamTapIn instance
  playback->owner = this;
  playback->composition_over_inheritance = base->instantiate_playback();
  
  return playback;
}

void AudioStreamTapInPlayback::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_event_count"), &AudioStreamTapInPlayback::get_event_count);
}

int AudioStreamTapInPlayback::mix(AudioFrame *p_buffer, float p_rate_scale, int p_frames) {
  p_frames = composition_over_inheritance->mix(p_buffer, p_rate_scale, p_frames);

  Ref<TapSim> simulator = owner->ls_in.get_simulator();
  tap_time_t target_time = current_time + (p_frames * p_rate_scale) * simulator->get_tick_rate();
  if (!owner->ls_in.try_lock(target_time, p_buffer, p_frames)) {
    return p_frames;
  }

  PackedInt64Array input_pids = owner->get_input_pids();

  for (int i = 0; i < p_frames; i++) {
    tap_time_t frame_time = current_time + (i * p_rate_scale) * simulator->get_tick_rate();
    for (tap_label_t pid : input_pids) {
      simulator->push_event(frame_time, p_buffer[i], pid);
    }
  }

  current_time = target_time;

  owner->ls_in.unlock();
  
  return p_frames;
}

void AudioStreamTapInPlayback::start(double p_from_pos) {
  owner->ls_in.set_live(true);
  if (owner->ls_in.get_live()) {
    current_time = owner->ls_in.get_simulator()->get_latest_event_time();
  }
}

void AudioStreamTapInPlayback::stop() {
  owner->ls_in.set_live(false);
}

bool AudioStreamTapInPlayback::is_playing() const {
  return owner->get_live();
}



