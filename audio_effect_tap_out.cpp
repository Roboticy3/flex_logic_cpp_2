#include "core/templates/hash_map.h"
#include "core/templates/vector.h"

#include "audio_effect_tap_out.h"
#include "audio_analyzer.h"

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
  ADD_PROPERTY(PropertyInfo(Variant::BOOL, "live"), "set_live", "get_live");
  ADD_PROPERTY(PropertyInfo(Variant::INT, "sample_skip", PROPERTY_HINT_ENUM_SUGGESTION, "1,2,4"), "set_sample_skip", "get_sample_skip");
}

bool AudioEffectTapOut::any_input_connected(PackedInt64Array input_pids) {
  return false;
}

Ref<TapSim> AudioEffectTapOut::get_simulator() const {
  return ls.get_simulator();
}

void AudioEffectTapOut::set_simulator(Ref<TapSim> new_simulator) {
  ls.set_simulator(new_simulator);
}

PackedInt64Array AudioEffectTapOut::get_output_pids() const {
  return ls.get_live_pids();
}

void AudioEffectTapOut::set_output_pids(PackedInt64Array new_output_pids) {
  ls.set_live_pids(new_output_pids);
}

bool AudioEffectTapOut::get_live() const {
  return ls.get_live();
}

void AudioEffectTapOut::set_live(bool new_live) {
  ls.set_live(new_live);
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
  if (effect->ls.get_live()) {
    process_live(p_src_frames, p_dst_frames, p_frame_count);
  }
}

bool AudioEffectTapOutInstance::process_silence() const {
  return true;
}

void AudioEffectTapOutInstance::process_live(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) {
  
  AudioAnalyzer analyzer;
  HashMap<tap_label_t, Vector<AudioFrame>> pid_readouts;

  //simulate the circuit
  //technically simulating behind by one `process` call because `total_time` 
  //  isn't updated yet. But if this instance is synced properly with the input
  //  instances, that's the only way to stop it from trying to read events that
  //  don't exist yet. Should cause a negligible delay.
  Ref<TapPatchBay> patch_bay = effect->ls.get_simulator()->get_patch_bay();
  for (int i = 0; i < p_frame_count; i += effect->sample_skip) {
    effect->ls.get_simulator()->process_to(total_time + i * effect->ls.get_simulator()->get_tick_rate());

    AudioFrame total;
    for (tap_label_t pid : effect->ls.get_live_pids()) {
      AudioFrame pid_frame = patch_bay->get_pin_frame(pid);

      pid_readouts[pid] = Vector<AudioFrame>();
      pid_readouts[pid].append(pid_frame);
      
      total += pid_frame;
    }
    p_dst_frames[i] = total;
  }

  for (auto pair : pid_readouts) {
    AudioAnalyzer::statistics_t stats = analyzer.stats(pair.value);
    print_line(pair.key, ": (", stats.mean.left, ", ", stats.mean.right, ") | (", stats.stddev.left, ", ", stats.stddev.right, ")");
  }

  total_time += p_frame_count * effect->ls.get_simulator()->get_tick_rate();
}