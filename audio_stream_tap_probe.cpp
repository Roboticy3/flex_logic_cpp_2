
#include "core/math/audio_frame.h"
#include "core/math/math_funcs.h"
#include "audio_stream_tap_probe.h"

void AudioStreamTapProbe::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_simulator"), &AudioStreamTapProbe::get_simulator);
  ClassDB::bind_method(D_METHOD("set_simulator", "sim"), &AudioStreamTapProbe::set_simulator);
  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "simulator", PROPERTY_HINT_RESOURCE_TYPE, "TapCircuit"), "set_simulator", "get_simulator");

  ClassDB::bind_method(D_METHOD("get_input_pids"), &AudioStreamTapProbe::get_input_pids);
  ClassDB::bind_method(D_METHOD("set_input_pids", "pids"), &AudioStreamTapProbe::set_input_pids);
  ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT64_ARRAY, "input_pids"), "set_input_pids", "get_input_pids");

  ClassDB::bind_method(D_METHOD("get_live"), &AudioStreamTapProbe::get_live);
  ADD_PROPERTY(PropertyInfo(Variant::BOOL, "live", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "", "get_live");
}

Ref<AudioStreamPlayback> AudioStreamTapProbe::instantiate_playback() {
  Ref<AudioStreamTapProbePlayback> new_playback;

  new_playback.instantiate();

	new_playback->stream = this;
	new_playback->mix_rate = AudioServer::get_singleton()->get_mix_rate();

  return new_playback;
}

void AudioStreamTapProbePlayback::_bind_methods() {
}

int AudioStreamTapProbePlayback::mix(AudioFrame *p_buffer, float p_rate_scale, int p_frames) {

  if (!stream->ls_in.try_lock(0, p_buffer, p_frames)) {
    return 0;
  }

  //generate a sine wave using the frame count and the mix rate combined with
  //the frequency

  double factor = (double)p_rate_scale * (double)frequency * 2 * Math::PI;

  for (int i = 0; i < p_frames; i++) {
    double value = Math::sin(phase + (double)i / (double)mix_rate * factor);
    p_buffer[i] = AudioFrame(value, value);
  }

  double time_elapsed = (double)p_frames / (double)mix_rate * factor;
  phase += time_elapsed;

  stream->ls_in.unlock();

  return p_frames;
}

float AudioStreamTapProbePlayback::get_stream_sampling_rate() {
  return mix_rate;
}

double AudioStreamTapProbePlayback::get_playback_position() const {
  return phase;
}

void AudioStreamTapProbePlayback::start(double start_time) {
  phase = start_time;
  stream->ls_in.set_live(true);
}

void AudioStreamTapProbePlayback::stop() {
  stream->ls_in.set_live(false);
  phase = 0.0;
}

bool AudioStreamTapProbePlayback::is_playing() const {
	return stream->ls_in.get_live();
}

int AudioStreamTapProbePlayback::get_loop_count() const {
	return 0;
}

void AudioStreamTapProbePlayback::seek(double p_time) {
}

void AudioStreamTapProbePlayback::tag_used_streams() {
}

AudioStreamTapProbePlayback::~AudioStreamTapProbePlayback() {
	stop();
}

AudioStreamTapProbePlayback::AudioStreamTapProbePlayback() {
}
