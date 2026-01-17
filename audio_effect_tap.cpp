
#include "core/object/class_db.h"
#include "core/math/audio_frame.h"
#include "core/math/math_defs.h"

#include "audio_effect_tap.h"
#include "tap_circuit_types.h"
#include "tap_patch_bay.h"
#include "tap_sim.h"

void AudioEffectTap::_bind_methods() {

  ClassDB::bind_method(D_METHOD("get_simulator"), &AudioEffectTap::get_simulator);
  ClassDB::bind_method(D_METHOD("set_simulator", "new_simulator"), &AudioEffectTap::set_simulator);

  ClassDB::bind_method(D_METHOD("get_activation_delta"), &AudioEffectTap::get_activation_delta);
  ClassDB::bind_method(D_METHOD("set_activation_delta", "new_activation_delta"), &AudioEffectTap::set_activation_delta);
  
  ClassDB::bind_method(D_METHOD("get_pid"), &AudioEffectTap::get_pid);
  ClassDB::bind_method(D_METHOD("set_pid", "new_pid"), &AudioEffectTap::set_pid);

  ClassDB::bind_method(D_METHOD("get_tick_rate"), &AudioEffectTap::get_tick_rate);
  ClassDB::bind_method(D_METHOD("set_tick_rate", "new_tick_rate"), &AudioEffectTap::set_tick_rate);

  ClassDB::bind_method(D_METHOD("get_line_in"), &AudioEffectTap::get_line_in);
  ClassDB::bind_method(D_METHOD("set_line_in", "new_line_in"), &AudioEffectTap::set_line_in);

  ClassDB::bind_method(D_METHOD("get_line_out"), &AudioEffectTap::get_line_out);
  ClassDB::bind_method(D_METHOD("set_line_out", "new_line_out"), &AudioEffectTap::set_line_out);

  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "simulator", PROPERTY_HINT_RESOURCE_TYPE, "TapSim"), "set_simulator", "get_simulator");
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "activation_delta", PROPERTY_HINT_RANGE, "0,65535"), "set_activation_delta", "get_activation_delta");
  ADD_PROPERTY(PropertyInfo(Variant::INT, "pid"), "set_pid", "get_pid");

  ADD_PROPERTY(PropertyInfo(Variant::INT, "tick_rate", PROPERTY_HINT_RANGE, "0,1024"), "set_tick_rate", "get_tick_rate");
  ADD_PROPERTY(PropertyInfo(Variant::BOOL, "line_in"), "set_line_in", "get_line_in");
  ADD_PROPERTY(PropertyInfo(Variant::BOOL, "line_out"), "set_line_out", "get_line_out");
}

Ref<AudioEffectInstance> AudioEffectTap::instantiate() {
  Ref<AudioEffectTapInstance> instance;
  instance.instantiate();
  instance->effect = this;
  return instance;
}

tap_frame::bytes_t AudioEffectTap::get_activation_delta() const {
  return activation_delta;
}

void AudioEffectTap::set_activation_delta(tap_frame::bytes_t new_activation_delta) {
  activation_delta = new_activation_delta;
}

Ref<TapSim> AudioEffectTap::get_simulator() const {
  return simulator;
}

void AudioEffectTap::set_simulator(Ref<TapSim> new_simulator) {
  simulator = new_simulator;

  if (simulator.is_null()) {
    simulator.instantiate();
  }
}

tap_label_t AudioEffectTap::get_pid() const {
  return pid;
}

void AudioEffectTap::set_pid(tap_label_t new_pid) {
  pid = new_pid;
}

int AudioEffectTap::get_tick_rate() const {
  return tick_rate;
}

void AudioEffectTap::set_tick_rate(int new_tick_rate) {
  tick_rate = new_tick_rate;
}

bool AudioEffectTap::get_line_in() const {
  return line_in;
}

void AudioEffectTap::set_line_in(bool new_line_in) {
  line_in = new_line_in;
}

bool AudioEffectTap::get_line_out() const {
  return line_out;
}

void AudioEffectTap::set_line_out(bool new_line_out) {
  line_out = new_line_out;
}

AudioEffectTap::AudioEffectTap() {}

void AudioEffectTapInstance::process(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) {
  if (effect->line_in) {
    process_line_in(p_src_frames, p_dst_frames, p_frame_count);
  }

  //process TapSim until the time passes by p_frame_count * tick_rate
  tap_time_t time_elapsed = effect->tick_rate * p_frame_count;

  if (effect->line_out) {
    //process output
  }

  total_time = time_elapsed;
}

void AudioEffectTapInstance::process_line_in(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) {

  Ref<TapPatchBay> patch_bay = effect->simulator->get_patch_bay();
  if (patch_bay.is_null()) {
    return;
  }

  tap_queue_t &queue = patch_bay->get_queue_internal();

  //tap_frame::bytes_t max_delta = 0;
  //tap_frame::bytes_t max_value = 0;
  for (int i = 0; i < p_frame_count; i++) {
    tap_frame src_frame = tap_frame(p_src_frames[i]);
    //max_delta = last_activation.delta(src_frame) > max_delta ? last_activation.delta(src_frame):max_delta;
    //max_value = src_frame.left > max_value ? src_frame.left : max_value;
    //max_value = src_frame.right > max_value ? src_frame.right : max_value;
    if (last_activation.delta(src_frame) >= effect->activation_delta) {
      tap_time_t time = total_time + i * effect->tick_rate;
      queue.insert({time, src_frame, effect->pid, patch_bay->COMPONENT_MISSING}, time);
      last_activation = src_frame;
    }
  }

  patch_bay->set_sample_count_internal(p_frame_count);
  total_time += p_frame_count * effect->tick_rate;

  //print_line(vformat("Max delta for audio process step: %d", max_delta));
  //print_line(vformat("Max VALUE for audio process step: %d", max_value));
}

bool AudioEffectTapInstance::process_silence() const {
  return true;
}