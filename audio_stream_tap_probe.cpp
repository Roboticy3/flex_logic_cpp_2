#include <iostream>

#include "core/math/audio_frame.h"
#include "core/math/math_funcs.h"
#include "audio_stream_tap_probe.h"

void AudioStreamTapProbe::_bind_methods() {
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

  if (!stream->ls_out.try_lock(0, p_buffer, p_frames)) {
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

  stream->ls_out.unlock();

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
  stream->ls_out.set_live(true);
}

void AudioStreamTapProbePlayback::stop() {
  stream->ls_out.set_live(false);
  phase = 0.0;
}

bool AudioStreamTapProbePlayback::is_playing() const {
	return stream->ls_out.get_live();
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
