#include "audio_stream_tap_simulator.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/string/print_string.h"
#include "core/variant/variant.h"
#include "servers/audio/audio_stream.h"
#include "tap_circuit_types.h"
#include "tap_patch_bay.h"
#include <iostream>
#include <mutex>

void AudioStreamTapSimulator::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_input_streams"), &AudioStreamTapSimulator::get_input_streams);
  ClassDB::bind_method(D_METHOD("set_input_streams", "streams"), &AudioStreamTapSimulator::set_input_streams);
  ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "input_streams", PROPERTY_HINT_ARRAY_TYPE, "TapInput", PROPERTY_USAGE_EDITOR), "set_input_streams", "get_input_streams");

  ClassDB::bind_method(D_METHOD("get_debug_input_override"), &AudioStreamTapSimulator::get_debug_input_override);
  ClassDB::bind_method(D_METHOD("set_debug_input_override", "label"), &AudioStreamTapSimulator::set_debug_input_override);
  ADD_PROPERTY(PropertyInfo(Variant::INT, "debug_input_override", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "set_debug_input_override", "get_debug_input_override");

  ClassDB::bind_method(D_METHOD("get_debug_reference_override"), &AudioStreamTapSimulator::get_debug_reference_override);
  ClassDB::bind_method(D_METHOD("set_debug_reference_override", "enabled"), &AudioStreamTapSimulator::set_debug_reference_override);
  ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_reference_override", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "set_debug_reference_override", "get_debug_reference_override");

  ClassDB::bind_method(D_METHOD("get_output_pids"), &AudioStreamTapSimulator::get_output_pids);
  ClassDB::bind_method(D_METHOD("set_output_pids", "pids"), &AudioStreamTapSimulator::set_output_pids);
  ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT64_ARRAY, "output_pids", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "set_output_pids", "get_output_pids");

  ClassDB::bind_method(D_METHOD("get_circuit"), &AudioStreamTapSimulator::get_circuit);
  ClassDB::bind_method(D_METHOD("set_circuit", "circuit"), &AudioStreamTapSimulator::set_circuit);
  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "circuit", PROPERTY_HINT_RESOURCE_TYPE, "TapCircuit"), "set_circuit", "get_circuit");

  ClassDB::bind_method(D_METHOD("get_reference_sim"), &AudioStreamTapSimulator::get_reference_sim);
  ClassDB::bind_method(D_METHOD("set_reference_sim", "reference_sim"), &AudioStreamTapSimulator::set_reference_sim);
  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "reference_sim", PROPERTY_HINT_RESOURCE_TYPE, "ReferenceSim"), "set_reference_sim", "get_reference_sim");

  ClassDB::bind_method(D_METHOD("get_tolerance"), &AudioStreamTapSimulator::get_tolerance);
  ClassDB::bind_method(D_METHOD("set_tolerance", "tolerance"), &AudioStreamTapSimulator::set_tolerance);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "tolerance"), "set_tolerance", "get_tolerance");

  ClassDB::bind_method(D_METHOD("get_sample_skip"), &AudioStreamTapSimulator::get_sample_skip);
  ClassDB::bind_method(D_METHOD("set_sample_skip", "sample_skip"), &AudioStreamTapSimulator::set_sample_skip);
  ADD_PROPERTY(PropertyInfo(Variant::INT, "sample_skip"), "set_sample_skip", "get_sample_skip");

  ClassDB::bind_method(D_METHOD("get_tick_rate"), &AudioStreamTapSimulator::get_tick_rate);
  ClassDB::bind_method(D_METHOD("set_tick_rate", "tick_rate"), &AudioStreamTapSimulator::set_tick_rate);
  ADD_PROPERTY(PropertyInfo(Variant::INT, "tick_rate"), "set_tick_rate", "get_tick_rate");

  ClassDB::bind_method(D_METHOD("get_live"), &AudioStreamTapSimulator::is_simulating);
  ADD_PROPERTY(PropertyInfo(Variant::BOOL, "live", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "", "get_live");

  ClassDB::bind_method(D_METHOD("can_simulate"), &AudioStreamTapSimulator::can_simulate);
  ClassDB::bind_method(D_METHOD("is_simulating"), &AudioStreamTapSimulator::is_simulating);
  ClassDB::bind_method(D_METHOD("is_circuit_correct"), &AudioStreamTapSimulator::is_circuit_correct);
  ClassDB::bind_method(D_METHOD("pop_back"), &AudioStreamTapSimulator::pop_back);

  ClassDB::bind_method(D_METHOD("get_playback", "pid"), &AudioStreamTapSimulator::get_playback);
  ClassDB::bind_method(D_METHOD("get_event_counts"), &AudioStreamTapSimulator::get_event_counts);
  ClassDB::bind_method(D_METHOD("get_loop_count", "pid"), &AudioStreamTapSimulator::get_loop_count);
}

TypedArray<Ref<TapInput>> AudioStreamTapSimulator::get_input_streams() const {
  TypedArray<Ref<TapInput>> array;
  for (const auto &kv : input_streams) {
    array.push_back(kv.value);
  }
  return array;
}

void AudioStreamTapSimulator::set_input_streams(const TypedArray<Ref<TapInput>> &p_inputs) {
  if (circuit.is_valid()) {
    circuit->get_mutex().lock();
  }
  
  input_streams.clear();
  for (int i = 0; i < p_inputs.size(); i++) {
    Ref<TapInput> input = p_inputs[i];
    if (input.is_valid()) {
      input_streams[input->get_pid()] = input;
    }
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

bool AudioStreamTapSimulator::get_debug_reference_override() const {
  if (circuit.is_valid()) {
    circuit->get_mutex().lock();
  }

  bool debug_reference_override_copy = debug_reference_override;

  if (circuit.is_valid()) {
    circuit->get_mutex().unlock();
  }

  return debug_reference_override_copy;
}

void AudioStreamTapSimulator::set_debug_reference_override(bool new_debug_reference_override) {
  if (circuit.is_valid()) {
    circuit->get_mutex().lock();
  }
  
  debug_reference_override = new_debug_reference_override;
  
  if (circuit.is_valid()) {
    circuit->get_mutex().unlock();
  }
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

float AudioStreamTapSimulator::get_tolerance() const {
  return tolerance; //tolerance doesn't interact with the circuit directly, so it doesn't need mutexing
}

void AudioStreamTapSimulator::set_tolerance(float new_tolerance) {
  tolerance = new_tolerance;
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

bool AudioStreamTapSimulator::is_circuit_correct() {
  auto frame = reference_frame_stack.top();
  if (frame.is_null()) {
    return false;
  }
  return frame->get_success();
}

Ref<ReferenceFrame> AudioStreamTapSimulator::pop_back() {
  auto result = reference_frame_stack.top();
  reference_frame_stack.pop();
  return result;
}

void AudioStreamTapSimulator::_stash_error() {
  Ref<ReferenceFrame> frame;
  frame.instantiate(reference_sim->get_total_error(), tolerance);
  reference_frame_stack.push(frame);
  reference_sim->reset();
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
    Ref<TapInput> input = kv.value;
    if (input.is_null() || input->get_stream().is_null()) {
      continue;
    }

    if (!circuit->get_patch_bay()->has_pin(input->get_pid())) {
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

Ref<AudioStreamPlayback> AudioStreamTapSimulator::get_playback(tap_label_t pid) const {
  if (!circuit.is_valid()) {
    return Ref<AudioStreamPlayback>();
  }

  std::lock_guard<std::recursive_mutex> lock(circuit->get_mutex());
  
  if (!trackers.has(pid)) {
    return Ref<AudioStreamPlayback>();
  }
  
  return trackers[pid].playback;
}

int AudioStreamTapSimulator::get_loop_count(tap_label_t pid) const {
  if (!circuit.is_valid()) {
    return -1;
  }

  std::lock_guard<std::recursive_mutex> lock(circuit->get_mutex());
  
  if (!trackers.has(pid)) {
    return -1;
  }
  
  return trackers[pid].inner_loop_count;
}

Ref<AudioStreamPlayback> AudioStreamTapSimulator::instantiate_playback() {

  trackers.clear();

  for (auto kv : input_streams) {
    Ref<TapInput> input = kv.value;
    if (!input.is_valid()) {
      continue;
    }

    Ref<AudioStream> stream = input->get_stream();
    if (!stream.is_valid()) {
      ERR_PRINT("Stream is not valid");
      return Ref<AudioStreamPlayback>();
    }

    double loop_length = Math::INF;
    if (stream->get_length() > 0 && input->get_inner_loop_count() > 0) {
      loop_length = (stream->get_length()) / (double)input->get_inner_loop_count();
    }
    trackers[input->get_pid()] = {stream->instantiate_playback(), 0, 0, loop_length, 0};
  }

  // Create a new instance of AudioStreamTapSimulatorPlayback
  Ref<AudioStreamTapSimulatorPlayback> playback;
  playback.instantiate();

  for (auto kv : input_streams) {
    Ref<TapInput> input = kv.value;
    playback->debug_input_pids.insert(input->get_pid());
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
    Ref<AudioStreamPlayback> playback = kv.value.playback;
    arr.push_back(playback.is_valid() ? playback->get_loop_count() : 0);
  }
  return arr;
}

void AudioStreamTapSimulatorPlayback::_bind_methods() {};

int AudioStreamTapSimulatorPlayback::mix_debug(tap_frame_t *p_buffer, float p_rate_scale, int p_frames) {
  if (!debug_input_pids.has(owner->debug_input_override)) {
    for (int i = 0; i < p_frames; i++) {
      p_buffer[i] = tap_frame_t(0, 0);
    }
    return p_frames;
  }

  Ref<AudioStreamPlayback> playback = owner->trackers[owner->debug_input_override].playback;

  if (!playback.is_valid()) {
    for (int i = 0; i < p_frames; i++) {
      p_buffer[i] = tap_frame_t(0, 0);
    }
    return p_frames;
  }

  //again, stealing this idea from how AudioStreamSynchronized is implemented
  //It looks like the intent is to use the fixed-size buffer to manage the streams
  //in a way that doesn't require dynamic memory.
  int todo = p_frames;
  tap_frame_t *rolling_buffer = p_buffer;

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

        int mixed = tracker.playback->mix(mix_buffer, p_rate_scale, to_mix);
        
        for (int j = 0; j < mixed; j += owner->sample_skip) {  
          //input circuit events here.
          tap_time_t time = rolling_time + (j * p_rate_scale) * owner->tick_rate;
          owner->circuit->push_event(time, mix_buffer[j], label);
          
          // Sync with reference circuit if available
          if (owner->reference_sim.is_valid() && owner->reference_sim->needs_tap()) {
            owner->reference_sim->get_circuit()->push_event(time, mix_buffer[j], label);
          }
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

int AudioStreamTapSimulatorPlayback::mix_out(tap_frame_t *p_buffer, float p_rate_scale, int p_frames) {
  //read out the simulator contents

  if (owner->output_pids.size() == 0) {
    return p_frames;
  }

  bool measure_error = owner->reference_sim.is_valid();
  bool use_debug_reference_override = owner->debug_reference_override;
  auto patch_bay = owner->circuit->get_patch_bay();

  for (int i = 0; i < p_frames; i++) {
    //compute the solution
    if (i % owner->sample_skip == 0) {
      // Convert HashMap to vector for indexed access
      LocalVector<Ref<TapInput>> input_vector;
      for (auto kv : owner->input_streams) {
        input_vector.push_back(kv.value);
      }
      
      for (size_t j = 0; j < MIN(input_vector.size(), problem.size()); j++) {
        problem[j] = patch_bay->get_pin_state(input_vector[j]->get_pid());
      }
      processed_events_count += owner->circuit->process_to(current_time + (i * p_rate_scale) * owner->tick_rate);
      
      // Sync with reference circuit if available
      if (owner->reference_sim.is_valid() && owner->reference_sim->needs_tap()) {
        owner->reference_sim->get_circuit()->process_to(current_time + (i * p_rate_scale) * owner->tick_rate);
      }
    
      //fill the solution buffer
      for (int j = 0; j < MIN(owner->output_pids.size(), solution.size()); j++) {
        solution[j] = patch_bay->get_pin_state_internal(owner->output_pids[j]);
      }

      //compute the problem/solution error
      if (measure_error) {
        //std::cout << "problem size " << problem.size() << " solution size " << solution.size() << std::endl;
        //std::cout << "plevels: " << problem[0].l << ", " << problem[1].l << "; err: " << solution[0].l - (problem[0].l + problem[1].l) << std::endl;
        owner->reference_sim->measure_error_internal(solution, problem, 1.0 / (mix_rate * (double)p_rate_scale));
      }
    }

    //fill the audio buffer
    for (size_t j = 0; j < solution.size(); j++) {
      //this line is still kind of ugly. Make sure to undo the references inside tapsim before merging fix-#19
      p_buffer[i] += use_debug_reference_override ? solution[j] : patch_bay->get_pin_state_internal(owner->output_pids[j]);
    }
  }
  return p_frames;
}

int AudioStreamTapSimulatorPlayback::mix_stats(tap_frame_t *p_buffer, float p_rate_scale, int p_frames) {
  
  tap_frame_t avg;

  for (int i = 0; i < p_frames; i++) {
    avg += p_buffer[i];
  }

  avg /= (float)p_frames;
    
  return p_frames;
}

bool AudioStreamTapSimulatorPlayback::mix_force_loop() {
  if (!owner->force_loop) {
    return false;
  }

  bool any_playback_stopped = false;
  bool any_playback_looped = false;
  
  // First pass: check for stopped playback (highest priority)
  for (auto &pair : owner->trackers) {
    AudioStreamTapSimulator::playback_tracker_t &tracker = pair.value;
    Ref<AudioStreamPlayback> playback = tracker.playback;
    if (playback.is_null()) {
      continue;
    }

    if (!playback->is_playing()) {
      any_playback_stopped = true;
      break; // Stop checking once we find a stopped playback
    }
  }

  // Second pass: only check for loops if nothing stopped
  if (!any_playback_stopped) {
    for (auto &pair : owner->trackers) {
      AudioStreamTapSimulator::playback_tracker_t &tracker = pair.value;
      Ref<AudioStreamPlayback> playback = tracker.playback;
      if (playback.is_null()) {
        continue;
      }

      double playback_postion = playback->get_playback_position();
      double next_inner_loop = (tracker.inner_loop_length * tracker.inner_loop_count + 1);

      if (tracker.inner_loop_length > 0 && playback_postion > next_inner_loop + INNER_LOOP_LENGTH_ROUNDING_ERROR) {
        any_playback_looped = true;
        tracker.inner_loop_count++;
      }
    }
  }

  if (any_playback_stopped || any_playback_looped) {
    print_line("Reference frame pushed! ", owner->reference_sim->get_total_error());
    owner->_stash_error();
  }

  if (any_playback_stopped) {
    stop();
    start();
  }
  
  return any_playback_stopped;
}

int AudioStreamTapSimulatorPlayback::mix(tap_frame_t *p_buffer, float p_rate_scale, int p_frames) {
  if (!playing ||!owner->circuit->get_mutex().try_lock()) {
    return 0;
  }

  mix_force_loop();

  mix_debug(p_buffer, p_rate_scale, p_frames);

  mix_in(p_rate_scale, p_frames);

  mix_out(p_buffer, p_rate_scale, p_frames);

  mix_stats(p_buffer, p_rate_scale, p_frames);

  current_time += (p_frames * p_rate_scale) * owner->tick_rate;

  owner->circuit->get_mutex().unlock();
  
  return p_frames;
}

void AudioStreamTapSimulatorPlayback::start(double p_from_pos) {
  current_time = 0.0;

  if (playing) {
    stop();
  }

  //these conditions don't work for some reason.
  if (owner->can_simulate()) {
    playing = true;
    for (auto kv : owner->trackers) {
      kv.value.event_count = 0;
      kv.value.inner_loop_count = 0;
      kv.value.last_playback_position = 0.0;

      kv.value.playback->start(p_from_pos);
    }
    owner->circuit->reset_live_states();
    
    // Sync with reference circuit if available
    if (owner->reference_sim.is_valid() && owner->reference_sim->needs_tap()) {
      owner->reference_sim->get_circuit()->reset_live_states();
    }
    
    processed_events_count = 0;
  } else {
    stop();
  }
}

void AudioStreamTapSimulatorPlayback::stop() {
  playing = false;
  
  if (owner->is_simulating()) {
    for (auto kv : owner->trackers) {
      kv.value.playback->stop();
    }
    owner->circuit->reset_live_states();
    
    // Sync with reference circuit if available
    if (owner->reference_sim.is_valid() && owner->reference_sim->needs_tap()) {
      owner->reference_sim->get_circuit()->reset_live_states();
    }
    
    processed_events_count = 0;
  }
}

bool AudioStreamTapSimulatorPlayback::is_playing() const {
  return owner->is_simulating();
}

