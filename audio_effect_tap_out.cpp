#include "audio_effect_tap_out.h"

void AudioEffectTapOut::_bind_methods() {
  ClassDB::bind_method(D_METHOD("any_input_connected"), &AudioEffectTapOut::any_input_connected);

  ClassDB::bind_method(D_METHOD("get_simulator"), &AudioEffectTapOut::get_simulator);
  ClassDB::bind_method(D_METHOD("set_simulator", "new_simulator"), &AudioEffectTapOut::set_simulator);

  ClassDB::bind_method(D_METHOD("get_output_pids"), &AudioEffectTapOut::get_output_pids);
  ClassDB::bind_method(D_METHOD("set_output_pids", "new_output_pids"), &AudioEffectTapOut::set_output_pids);

  ClassDB::bind_method(D_METHOD("get_live"), &AudioEffectTapOut::get_live);
  ClassDB::bind_method(D_METHOD("set_live", "new_live"), &AudioEffectTapOut::set_live);

  ClassDB::bind_method(D_METHOD("get_sample_skip"), &AudioEffectTapOut::get_sample_skip);
  ClassDB::bind_method(D_METHOD("set_sample_skip", "new_sample_skip"), &AudioEffectTapOut::set_sample_skip);

  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "simulator", PROPERTY_HINT_RESOURCE_TYPE, "TapSim"), "set_simulator", "get_simulator");
  ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT64_ARRAY, "output_pids"), "set_output_pids", "get_output_pids");
  ADD_PROPERTY(PropertyInfo(Variant::BOOL, "live"), "get_live", "set_live");
  ADD_PROPERTY(PropertyInfo(Variant::INT, "sample_skip", PROPERTY_HINT_ENUM_SUGGESTION, "1,2,4"), "set_sample_skip", "get_sample_skip");
}

bool AudioEffectTapOut::any_input_connected(PackedInt64Array input_pids) {
  return false;
}

Ref<TapSim> AudioEffectTapOut::get_simulator() const {
  return simulator;
}
void AudioEffectTapOut::set_simulator(Ref<TapSim> new_simulator) {
  simulator = new_simulator;

  //invalidate the simulation.
  //not sure if this will validate properly in the editor.
  live = false;
  output_pids.clear();
}

PackedInt64Array AudioEffectTapOut::get_output_pids() const {
  return output_pids;
}
void AudioEffectTapOut::set_output_pids(PackedInt64Array new_output_pids) {
  //assume nothing is valid if no patch bay exists to validate pids.
  if (simulator.is_null() || simulator->get_patch_bay().is_null()) {
    output_pids.clear();
    return;
  }

  //use to validate pins
  Ref<TapPatchBay> patch_bay = simulator->get_patch_bay();

  output_pids.clear();
  
  for (int i = 0; i < new_output_pids.size(); ) {
    auto pid = new_output_pids[i];
    if (patch_bay->has_pin(pid)) {
      output_pids.append(pid);
    }
  }
}

bool AudioEffectTapOut::get_live() const {
  return live;
}
void AudioEffectTapOut::set_live(bool new_live) {
  if (new_live && output_pids.is_empty()) {
    ERR_PRINT("Cannot set live to true when output_pids is empty. Ensure output_pids points to valid pids in the simulator.");
    return;
  }

  live = new_live;
}

int AudioEffectTapOut::get_sample_skip() const {
  return sample_skip;
}
void AudioEffectTapOut::set_sample_skip(int new_sample_skip) {
  if (new_sample_skip < 1 || 512 % new_sample_skip != 0) {
    WARN_PRINT("Suggested sample skip does not divide 512, the typical batch count for audio processing. Expect phase issues.");
  }

  if (new_sample_skip > 4) {
    WARN_PRINT("Suggested sample skip would produce audio at a resolution under 10kHz at typical 44kHz audio processing. Expect choppiness");
  }

  sample_skip = new_sample_skip;
}

Ref<AudioEffectInstance> AudioEffectTapOut::instantiate() {
  Ref<AudioEffectTapOutInstance> instance;
  instance.instantiate();
  instance->effect = this;
  return instance;
}

void AudioEffectTapOutInstance::process(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) {
  if (effect->live) {
    process_live(p_src_frames, p_dst_frames, p_frame_count);
  }
}

bool AudioEffectTapOutInstance::process_silence() const {
  return true;
}

void AudioEffectTapOutInstance::process_live(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) {
  //simulate the circuit
  //technically simulating behind by one `process` call because `total_time` 
  //  isn't updated yet. But if this instance is synced properly with the input
  //  instances, that's the only way to stop it from trying to read events that
  //  don't exist yet. Should cause a negligible delay.
  for (int i = 0; i < p_frame_count; i += effect->sample_skip) {
    effect->simulator->process_to(total_time);
  }  

  //sum over the outputs. Not sure what to do about normalization right now.
  Ref<TapPatchBay> patch_bay = effect->simulator->get_patch_bay();
  for (int i = 0; i < p_frame_count; i += effect->sample_skip) {
    AudioFrame total;
    for (tap_label_t pid : effect->output_pids) {
      total += patch_bay->get_pin_frame(pid);
    }
    p_dst_frames[i] = total;
  }

  total_time += p_frame_count * effect->simulator->get_tick_rate();
}