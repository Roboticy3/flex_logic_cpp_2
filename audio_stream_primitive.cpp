#include <mutex>

#include "core/math/audio_frame.h"
#include "core/math/math_funcs.h"
#include "audio_stream_primitive.h"

void AudioStreamPrimitive::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_frequency"), &AudioStreamPrimitive::get_frequency);
  ClassDB::bind_method(D_METHOD("set_frequency", "frequency"), &AudioStreamPrimitive::set_frequency);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "frequency"), "set_frequency", "get_frequency");

  ClassDB::bind_method(D_METHOD("get_amplitude"), &AudioStreamPrimitive::get_amplitude);
  ClassDB::bind_method(D_METHOD("set_amplitude", "amplitude"), &AudioStreamPrimitive::set_amplitude);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "amplitude"), "set_amplitude", "get_amplitude");

  ClassDB::bind_method(D_METHOD("get_sin"), &AudioStreamPrimitive::get_sin);
  ClassDB::bind_method(D_METHOD("set_sin", "sin"), &AudioStreamPrimitive::set_sin);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sin"), "set_sin", "get_sin");

  ClassDB::bind_method(D_METHOD("get_tri"), &AudioStreamPrimitive::get_tri);
  ClassDB::bind_method(D_METHOD("set_tri", "tri"), &AudioStreamPrimitive::set_tri);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "tri"), "set_tri", "get_tri");

  ClassDB::bind_method(D_METHOD("get_sqr"), &AudioStreamPrimitive::get_sqr);
  ClassDB::bind_method(D_METHOD("set_sqr", "sqr"), &AudioStreamPrimitive::set_sqr);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sqr"), "set_sqr", "get_sqr");

  ClassDB::bind_method(D_METHOD("get_saw"), &AudioStreamPrimitive::get_saw);
  ClassDB::bind_method(D_METHOD("set_saw", "saw"), &AudioStreamPrimitive::set_saw);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "saw"), "set_saw", "get_saw");
}

float AudioStreamPrimitive::get_frequency() const {
  std::lock_guard<mutex_t> lock(audio_playback_mutex);
  return frequency;
}

void AudioStreamPrimitive::set_frequency(float p_frequency) {
  std::lock_guard<mutex_t> lock(audio_playback_mutex);
  frequency = p_frequency;
}

float AudioStreamPrimitive::get_amplitude() const {
  std::lock_guard<mutex_t> lock(audio_playback_mutex);
  return CLAMP(amplitude, 0.0f, 1.0f);
}

void AudioStreamPrimitive::set_amplitude(float p_amplitude) {
  std::lock_guard<mutex_t> lock(audio_playback_mutex);
  amplitude = p_amplitude;
}

float AudioStreamPrimitive::get_sin() const {
  std::lock_guard<mutex_t> lock(audio_playback_mutex);
  return sin;
}

void AudioStreamPrimitive::set_sin(float p_sin) {
  std::lock_guard<mutex_t> lock(audio_playback_mutex);
  sin = p_sin;
  normalize();
}

float AudioStreamPrimitive::get_tri() const {
  std::lock_guard<mutex_t> lock(audio_playback_mutex);
  return tri;
}

void AudioStreamPrimitive::set_tri(float p_tri) {
  std::lock_guard<mutex_t> lock(audio_playback_mutex);
  tri = p_tri;
  normalize();
}

float AudioStreamPrimitive::get_sqr() const {
  std::lock_guard<mutex_t> lock(audio_playback_mutex);
  return sqr;
}

void AudioStreamPrimitive::set_sqr(float p_sqr) {
  std::lock_guard<mutex_t> lock(audio_playback_mutex);
  sqr = p_sqr;
  normalize();
}

float AudioStreamPrimitive::get_saw() const {
  std::lock_guard<mutex_t> lock(audio_playback_mutex);
  return saw;
}

void AudioStreamPrimitive::set_saw(float p_saw) {
  std::lock_guard<mutex_t> lock(audio_playback_mutex);
  saw = p_saw;
  normalize();
}

void AudioStreamPrimitive::normalize() {
  float total = sin + tri + sqr + saw;
  if (total > 0.0f) {
    sin /= total;
    tri /= total;
    sqr /= total;
    saw /= total;
  }
}

Ref<AudioStreamPlayback> AudioStreamPrimitive::instantiate_playback() {

  Ref<AudioStreamPrimitivePlayback> new_playback;
  new_playback.instantiate();

  new_playback->stream = this;
  new_playback->mix_rate = AudioServer::get_singleton()->get_mix_rate();

  return new_playback;
}

void AudioStreamPrimitivePlayback::_bind_methods() {
}

int AudioStreamPrimitivePlayback::mix(AudioFrame *p_buffer, float p_rate_scale, int p_frames) {
  //use mutex to access stream data
  if (!stream->audio_playback_mutex.try_lock()) {
    return 0;
  }

  // Get stream settings
  double freq = stream->frequency;
  float amp = stream->amplitude;
  float sin_weight = stream->sin;
  float tri_weight = stream->tri;
  float sqr_weight = stream->sqr;
  float saw_weight = stream->saw;

  stream->audio_playback_mutex.unlock();

  if (!playing || p_frames <= 0) {
    return 0;
  }

  double phase_increment = (freq * p_rate_scale) / (double)mix_rate;
  
  for (int i = 0; i < p_frames; i++) {
    double sample = 0.0;
    
    // Generate sine wave
    if (sin_weight > 0.0f) {
      sample += sin_weight * Math::sin(phase * 2.0 * Math::PI);
    }
    
    // Generate triangle wave
    if (tri_weight > 0.0f) {
      double tri_phase = phase - Math::floor(phase);
      double tri_value = (tri_phase < 0.5) ? (4.0 * tri_phase - 1.0) : (3.0 - 4.0 * tri_phase);
      sample += tri_weight * tri_value;
    }
    
    // Generate square wave
    if (sqr_weight > 0.0f) {
      double sqr_value = (phase - Math::floor(phase) < 0.5) ? 1.0 : -1.0;
      sample += sqr_weight * sqr_value;
    }
    
    // Generate sawtooth wave
    if (saw_weight > 0.0f) {
      double saw_value = 2.0 * (phase - Math::floor(phase)) - 1.0;
      sample += saw_weight * saw_value;
    }
    
    // Apply amplitude and write to buffer
    sample *= amp;
    p_buffer[i] = AudioFrame(sample, sample);
    
    // Advance phase
    phase += phase_increment;
    if (phase >= 1.0) {
      phase -= Math::floor(phase);
    }
  }

  return p_frames;
}

float AudioStreamPrimitivePlayback::get_stream_sampling_rate() {
  return mix_rate;
}

double AudioStreamPrimitivePlayback::get_playback_position() const {
  return phase;
}

void AudioStreamPrimitivePlayback::start(double start_time) {
  phase = start_time;
  playing = true;
}

void AudioStreamPrimitivePlayback::stop() {
  playing = false;
  phase = 0.0;
}

bool AudioStreamPrimitivePlayback::is_playing() const {
	return playing;
}

int AudioStreamPrimitivePlayback::get_loop_count() const {
	return 0;
}

void AudioStreamPrimitivePlayback::seek(double p_time) {
}

void AudioStreamPrimitivePlayback::tag_used_streams() {
}

AudioStreamPrimitivePlayback::~AudioStreamPrimitivePlayback() {
	stop();
}

AudioStreamPrimitivePlayback::AudioStreamPrimitivePlayback() {
}
