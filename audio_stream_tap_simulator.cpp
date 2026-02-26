#include "audio_stream_tap_simulator.h"
#include "core/object/object.h"
#include "core/variant/variant.h"
#include "servers/audio/audio_stream.h"
#include "tap_circuit_types.h"
#include "tap_patch_bay.h"
#include <iostream>
#include <mutex>

void AudioStreamTapSimulator::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_input_streams"), &AudioStreamTapSimulator::get_input_streams);
  ClassDB::bind_method(D_METHOD("set_input_streams", "streams"), &AudioStreamTapSimulator::set_input_streams);
  ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "input_streams", PROPERTY_HINT_DICTIONARY_TYPE, "int:AudioStream", PROPERTY_USAGE_EDITOR), "set_input_streams", "get_input_streams");

  ClassDB::bind_method(D_METHOD("get_debug_input_override"), &AudioStreamTapSimulator::get_debug_input_override);
  ClassDB::bind_method(D_METHOD("set_debug_input_override", "label"), &AudioStreamTapSimulator::set_debug_input_override);
  ADD_PROPERTY(PropertyInfo(Variant::INT, "debug_input_override", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "set_debug_input_override", "get_debug_input_override");

  ClassDB::bind_method(D_METHOD("get_output_pids"), &AudioStreamTapSimulator::get_output_pids);
  ClassDB::bind_method(D_METHOD("set_output_pids", "pids"), &AudioStreamTapSimulator::set_output_pids);
  ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT64_ARRAY, "output_pids", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "set_output_pids", "get_output_pids");

  ClassDB::bind_method(D_METHOD("get_circuit"), &AudioStreamTapSimulator::get_circuit);
  ClassDB::bind_method(D_METHOD("set_circuit", "circuit"), &AudioStreamTapSimulator::set_circuit);
  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "circuit", PROPERTY_HINT_RESOURCE_TYPE, "TapCircuit"), "set_circuit", "get_circuit");

  ClassDB::bind_method(D_METHOD("get_reference_sim"), &AudioStreamTapSimulator::get_reference_sim);
  ClassDB::bind_method(D_METHOD("set_reference_sim", "reference_sim"), &AudioStreamTapSimulator::set_reference_sim);
  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "reference_sim", PROPERTY_HINT_RESOURCE_TYPE, "ReferenceSim"), "set_reference_sim", "get_reference_sim");

  ClassDB::bind_method(D_METHOD("get_sample_skip"), &AudioStreamTapSimulator::get_sample_skip);
  ClassDB::bind_method(D_METHOD("set_sample_skip", "sample_skip"), &AudioStreamTapSimulator::set_sample_skip);
  ADD_PROPERTY(PropertyInfo(Variant::INT, "sample_skip"), "set_sample_skip", "get_sample_skip");

  ClassDB::bind_method(D_METHOD("get_tick_rate"), &AudioStreamTapSimulator::get_tick_rate);
  ClassDB::bind_method(D_METHOD("set_tick_rate", "tick_rate"), &AudioStreamTapSimulator::set_tick_rate);
  ADD_PROPERTY(PropertyInfo(Variant::INT, "tick_rate"), "set_tick_rate", "get_tick_rate");

  ClassDB::bind_method(D_METHOD("get_live"), &AudioStreamTapSimulator::is_simulating);
  ADD_PROPERTY(PropertyInfo(Variant::BOOL, "live", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "", "get_live");

  ClassDB::bind_method(D_METHOD("get_event_counts"), &AudioStreamTapSimulator::get_event_counts);
  ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT64_ARRAY, "event_counts"), "", "get_event_counts");
}

TypedDictionary<tap_label_t, Ref<AudioStream>> AudioStreamTapSimulator::get_input_streams() const {
  TypedDictionary<tap_label_t, Ref<AudioStream>> dict;
  for (const auto &kv : input_streams) {
    dict.set(kv.pid, kv.stream);
  }
  return dict;
}

void AudioStreamTapSimulator::set_input_streams(const TypedDictionary<tap_label_t, Ref<AudioStream>> &p_streams) {
  if (circuit.is_valid()) {
    circuit->get_mutex().lock();
  }
  
  input_streams.clear();
  for (auto kv : p_streams) {
    input_streams.push_back({kv.value, kv.key});
  }
  
  if (circuit.is_valid()) {
    circuit->get_mutex().unlock();
  }
}

PackedInt64Array AudioStreamTapSimulator::get_output_pids() const {
  if (circuit.is_valid()) {
    circuit->get_mutex().lock();
  }

  PackedInt64Array output_pids_copy(output_pids);

  if (circuit.is_valid()) {
    circuit->get_mutex().unlock();
  }
  return output_pids_copy;
}

void AudioStreamTapSimulator::set_debug_input_override(tap_label_t new_debug_input_override) {
  if (circuit.is_valid()) {
    circuit->get_mutex().lock();
  }
  
  debug_input_override = new_debug_input_override;
  
  if (circuit.is_valid()) {
    circuit->get_mutex().unlock();
  }
}

tap_label_t AudioStreamTapSimulator::get_debug_input_override() const {
  if (circuit.is_valid()) {
    circuit->get_mutex().lock();
  }

  tap_label_t debug_input_override_copy = debug_input_override;

  if (circuit.is_valid()) {
    circuit->get_mutex().unlock();
  }

  return debug_input_override_copy;
}

void AudioStreamTapSimulator::set_output_pids(const PackedInt64Array &new_output_pids) {
  if (circuit.is_valid()) {
    circuit->get_mutex().lock();
  }
  
  output_pids = new_output_pids;
  
  if (circuit.is_valid()) {
    circuit->get_mutex().unlock();
  }
}

Ref<TapCircuit> AudioStreamTapSimulator::get_circuit() const {
  if (circuit.is_valid()) {
    circuit->get_mutex().lock();
  }
  
  Ref<TapCircuit> circuit_copy = circuit;
  
  if (circuit.is_valid()) {
    circuit->get_mutex().unlock();
  }
  return circuit_copy;
}

void AudioStreamTapSimulator::set_circuit(Ref<TapCircuit> new_circuit) {
  if (circuit.is_valid()) {
    circuit->get_mutex().lock();
  }
  
  auto old_circuit_copy = circuit;
  circuit = new_circuit;
  
  if (old_circuit_copy.is_valid()) {
    old_circuit_copy->get_mutex().unlock();
  }
}

Ref<ReferenceSim> AudioStreamTapSimulator::get_reference_sim() const {
  if (circuit.is_valid()) {
    circuit->get_mutex().lock();
  }
  
  Ref<ReferenceSim> reference_sim_copy = reference_sim;
  
  if (circuit.is_valid()) {
    circuit->get_mutex().unlock();
  }
  return reference_sim_copy;
}

void AudioStreamTapSimulator::set_reference_sim(Ref<ReferenceSim> new_reference_sim) {
  if (circuit.is_valid()) {
    circuit->get_mutex().lock();
  }
  
  reference_sim = new_reference_sim;
  
  if (circuit.is_valid()) {
    circuit->get_mutex().unlock();
  }
}

int AudioStreamTapSimulator::get_tick_rate() const {
  if (circuit.is_valid()) {
    circuit->get_mutex().lock();
  }
  
  int tick_rate_copy = tick_rate;
  
  if (circuit.is_valid()) {
    circuit->get_mutex().unlock();
  }
  return tick_rate_copy;
}

void AudioStreamTapSimulator::set_tick_rate(int new_tick_rate) {
  if (circuit.is_valid()) {
    circuit->get_mutex().lock();
  }
  
  tick_rate = new_tick_rate;
  
  if (circuit.is_valid()) {
    circuit->get_mutex().unlock();
  }
}

int AudioStreamTapSimulator::get_sample_skip() const {
  if (circuit.is_valid()) {
    circuit->get_mutex().lock();
  }

  int sample_skip_copy = sample_skip;

  if (circuit.is_valid()) {
    circuit->get_mutex().unlock();
  }

  return sample_skip_copy;
}

void AudioStreamTapSimulator::set_sample_skip(int new_sample_skip) {
  if (circuit.is_valid()) {
    circuit->get_mutex().lock();
  }
  
  sample_skip = new_sample_skip;

  if (circuit.is_valid()) {
    circuit->get_mutex().unlock();
  }
}

bool AudioStreamTapSimulator::is_simulating() const {
  if (circuit.is_valid()) {
    circuit->get_mutex().lock();
  }

  bool live = false;
  for (auto kv : trackers) {
    if (kv.value.playback->is_playing()) {
      live = true;
      break;
    }
  }

  if (circuit.is_valid()) {
    circuit->get_mutex().unlock();
  }

  return live;
}

bool AudioStreamTapSimulator::can_simulate() const {
  if (circuit.is_null()) {
    return false;
  }

  std::lock_guard<std::recursive_mutex> lock(circuit->get_mutex());

  if (!circuit->is_instantiated()) {
    return false;
  }

  for (auto kv : input_streams) {
    if (kv.stream.is_null()) {
      continue;
    }

    if (!circuit->get_patch_bay()->has_pin(kv.pid)) {
      return false;
    }
  }

  for (int64_t pid : output_pids) {
    if (!circuit->get_patch_bay()->has_pin(pid)) {
      return false;
    }
  }

  return true;
}

Ref<AudioStreamPlayback> AudioStreamTapSimulator::instantiate_playback() {

  trackers.clear();

  for (auto kv : input_streams) {
    Ref<AudioStream> stream = kv.stream;
    if (!stream.is_valid()) {
      ERR_PRINT("Stream is not valid");
      return Ref<AudioStreamPlayback>();
    }

    trackers[kv.pid] = {stream->instantiate_playback(), 0};
  }

  // Create a new instance of AudioStreamTapSimulatorPlayback
  Ref<AudioStreamTapSimulatorPlayback> playback;
  playback.instantiate();

  for (auto kv : input_streams) {
    playback->debug_input_pids.insert(kv.pid);
  }

  playback->problem.resize(input_streams.size());
  playback->solution.resize(output_pids.size());  
  
  // Set the owner of the playback to this AudioStreamTapSimulator instance
  playback->owner = this;
  
  return playback;
}

PackedInt64Array AudioStreamTapSimulator::get_event_counts() const {
  if (!circuit.is_valid()) {
    return PackedInt64Array();
  }

  std::lock_guard<std::recursive_mutex> lock(circuit->get_mutex());
  
  PackedInt64Array arr;
  for (auto kv: trackers) {
    arr.push_back(kv.value.event_count);
  }
  return arr;
}

void AudioStreamTapSimulatorPlayback::_bind_methods() {};

int AudioStreamTapSimulatorPlayback::mix_debug(AudioFrame *p_buffer, float p_rate_scale, int p_frames) {
  if (!debug_input_pids.has(owner->debug_input_override)) {
    for (int i = 0; i < p_frames; i++) {
      p_buffer[i] = AudioFrame(0, 0);
    }
    return p_frames;
  }

  Ref<AudioStreamPlayback> playback = owner->trackers[owner->debug_input_override].playback;

  if (!playback.is_valid()) {
    for (int i = 0; i < p_frames; i++) {
      p_buffer[i] = AudioFrame(0, 0);
    }
    return p_frames;
  }

  //again, stealing this idea from how AudioStreamSynchronized is implemented
  //It looks like the intent is to use the fixed-size buffer to manage the streams
  //in a way that doesn't require dynamic memory.
  int todo = p_frames;
  AudioFrame *rolling_buffer = p_buffer;

  while (todo) {
    int to_mix = MIN(todo, MIX_BUFFER_SIZE);
    playback->mix(rolling_buffer, p_rate_scale, to_mix);
    todo -= to_mix;
    rolling_buffer += to_mix;
  }

  return p_frames;
}

int AudioStreamTapSimulatorPlayback::mix_in(float p_rate_scale, int p_frames) {

  if (debug_input_pids.has(owner->debug_input_override)) {
    return p_frames;
  }

	int todo = p_frames;
  int rolling_time = current_time;

	bool any_active = false;
	while (todo) {
		int to_mix = MIN(todo, MIX_BUFFER_SIZE);

		bool first = true;
		for (auto &kv : owner->trackers) {
      tap_label_t label = kv.key;
			auto &tracker = kv.value;
			if (tracker.playback->is_playing()) {
				
        //TODO: volume controls for input streams

				if (first) {
					any_active = true;
				}

        tracker.playback->mix(mix_buffer, p_rate_scale, to_mix);
        for (int j = 0; j < to_mix; j += owner->sample_skip) {  
          //input circuit events here.
          tap_time_t time = rolling_time + (j * p_rate_scale) * owner->tick_rate;
          owner->circuit->push_event(time, mix_buffer[j], label);
        }

        tracker.event_count += to_mix / owner->sample_skip;

        //std::cout << "\tincrementing " << tracker.playback.ptr() << " to " << tracker.event_count << std::endl;
			}
		}
		todo -= to_mix;

    //update rolling time so the phase of the circuit is correct
    rolling_time += (to_mix * p_rate_scale) * owner->tick_rate;
	}

	if (!any_active) {
		stop();
    ERR_PRINT("AudioStreamTapSimulatorPlayback::mix_in tried to push events with no active input streams.");
	}
	return p_frames;
}

int AudioStreamTapSimulatorPlayback::mix_out(AudioFrame *p_buffer, float p_rate_scale, int p_frames) {
  //read out the simulator contents

  if (owner->output_pids.size() == 0) {
    return p_frames;
  }

  bool use_reference = owner->reference_sim.is_valid();
  auto patch_bay = owner->circuit->get_patch_bay();

  for (int i = 0; i < p_frames; i++) {

    //fill the problem buffer
    for (size_t j = 0; j < MIN(owner->input_streams.size(), problem.size()); j++) {
      problem[j] = patch_bay->get_pin_state(owner->input_streams[j].pid);
    }

    //compute the solution
    if (i % owner->sample_skip == 0) {
      processed_events_count += owner->circuit->process_to(current_time + (i * p_rate_scale) * owner->tick_rate);
    }

    //zero out the buffer before summing to avoid noise from previous frames
    p_buffer[i] = AudioFrame(0, 0);

    //fill the solution buffer
    for (int j = 0; j < MIN(owner->output_pids.size(), solution.size()); j++) {
      solution[j] = patch_bay->get_pin_state_internal(owner->output_pids[j]);
    }

    //compute the problem/solution error
    if (use_reference && i % owner->sample_skip == 0) {
      owner->reference_sim->measure_error_internal(solution, problem, 1.0 / (mix_rate * (double)p_rate_scale));
    }

    //fill the audio buffer
    for (size_t j = 0; j < solution.size(); j++) {
      //this line is still kind of ugly. Make sure to undo the references inside tapsim before merging fix-#19
      p_buffer[i] += solution[j];
    }
  }
  return p_frames;
}

int AudioStreamTapSimulatorPlayback::mix_stats(AudioFrame *p_buffer, float p_rate_scale, int p_frames) {
  
  AudioFrame avg;

  for (int i = 0; i < p_frames; i++) {
    avg += p_buffer[i];
  }

  avg /= (float)p_frames;

  std::cout << "AudioStreamTapSimulatorPlayback::mix_stats: " << avg.left << ", " << avg.right << std::endl;
  
  return p_frames;
}

int AudioStreamTapSimulatorPlayback::mix(AudioFrame *p_buffer, float p_rate_scale, int p_frames) {
  if (!owner->circuit->get_mutex().try_lock()) {
    return p_frames;
  }

  mix_debug(p_buffer, p_rate_scale, p_frames);

  mix_in(p_rate_scale, p_frames);

  mix_out(p_buffer, p_rate_scale, p_frames);

  mix_stats(p_buffer, p_rate_scale, p_frames);

  current_time += (p_frames * p_rate_scale) * owner->tick_rate;

  owner->circuit->get_mutex().unlock();
  
  return p_frames;
}

void AudioStreamTapSimulatorPlayback::start(double p_from_pos) {
  if (owner->can_simulate()) {
    current_time = 0.0;

    for (auto kv : owner->trackers) {
      kv.value.event_count = 0;
      kv.value.playback->start(p_from_pos);
    }
  }
}

void AudioStreamTapSimulatorPlayback::stop() {
  if (owner->is_simulating()) {
    for (auto kv : owner->trackers) {
      kv.value.playback->stop();
    }
  }
}

bool AudioStreamTapSimulatorPlayback::is_playing() const {
  return owner->is_simulating();
}

