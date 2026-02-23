#include "audio_stream_tap_simulator.h"
#include "core/object/object.h"
#include "core/variant/variant.h"
#include "servers/audio/audio_stream.h"
#include "tap_circuit_types.h"
#include <iostream>
#include <mutex>

void AudioStreamTapSimulator::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_input_streams"), &AudioStreamTapSimulator::get_input_streams);
  ClassDB::bind_method(D_METHOD("set_input_streams", "streams"), &AudioStreamTapSimulator::set_input_streams);
  ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "input_streams", PROPERTY_HINT_DICTIONARY_TYPE, "int:AudioStream", PROPERTY_USAGE_EDITOR), "set_input_streams", "get_input_streams");

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
  for (const KeyValue<tap_label_t, Ref<AudioStream>> &kv : input_streams) {
    dict.set(kv.key, kv.value);
  }
  return dict;
}

void AudioStreamTapSimulator::set_input_streams(const TypedDictionary<tap_label_t, Ref<AudioStream>> &p_streams) {
  input_streams.clear();
  for (auto kv : p_streams) {
    input_streams[kv.key] = kv.value;
  }
}

PackedInt64Array AudioStreamTapSimulator::get_output_pids() const {
  return output_pids;
}

void AudioStreamTapSimulator::set_output_pids(const PackedInt64Array &new_output_pids) {
  output_pids = new_output_pids;
}

Ref<TapCircuit> AudioStreamTapSimulator::get_circuit() const {
  return circuit;
}

void AudioStreamTapSimulator::set_circuit(Ref<TapCircuit> new_circuit) {
  circuit = new_circuit;
}

Ref<ReferenceSim> AudioStreamTapSimulator::get_reference_sim() const {
  return reference_sim;
}

void AudioStreamTapSimulator::set_reference_sim(Ref<ReferenceSim> new_reference_sim) {
  reference_sim = new_reference_sim;
}

int AudioStreamTapSimulator::get_tick_rate() const {
  return tick_rate;
}

void AudioStreamTapSimulator::set_tick_rate(int new_tick_rate) {
  tick_rate = new_tick_rate;
}

int AudioStreamTapSimulator::get_sample_skip() const {
  return sample_skip;
}

void AudioStreamTapSimulator::set_sample_skip(int new_sample_skip) {
  sample_skip = new_sample_skip;
}

bool AudioStreamTapSimulator::is_simulating() const {
  bool live = false;
  for (auto kv : trackers) {
    if (kv.value.playback->is_playing()) {
      live = true;
      break;
    }
  }
  return live;
}

bool AudioStreamTapSimulator::can_simulate() const {
  if (circuit.is_null()) {
    return false;
  }

  if (!circuit->is_instantiated()) {
    return false;
  }

  for (auto kv : input_streams) {
    if (kv.value.is_null()) {
      continue;
    }

    if (!circuit->get_patch_bay()->has_pin(kv.key)) {
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
    Ref<AudioStream> stream = kv.value;
    if (!stream.is_valid()) {
      ERR_PRINT("Stream is not valid");
      return Ref<AudioStreamPlayback>();
    }

    trackers[kv.key] = {stream->instantiate_playback(), 0};
  }

  // Create a new instance of AudioStreamTapSimulatorPlayback
  Ref<AudioStreamTapSimulatorPlayback> playback;
  playback.instantiate();
  
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

int AudioStreamTapSimulatorPlayback::mix_in(float p_rate_scale, int p_frames) {
	int todo = p_frames;

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
          tap_time_t time = current_time + (j * p_rate_scale) * owner->tick_rate;
          owner->circuit->push_event(time, mix_buffer[j], label);
        }

        tracker.event_count += to_mix;

        //std::cout << "\tincrementing " << tracker.playback.ptr() << " to " << tracker.event_count << std::endl;
			}
		}
		todo -= to_mix;
	}

	if (!any_active) {
		stop();
    ERR_PRINT("AudioStreamTapSimulatorPlayback::mix_in tried to push events with no active input streams.");
	}
	return p_frames;
}

int AudioStreamTapSimulatorPlayback::mix(AudioFrame *p_buffer, float p_rate_scale, int p_frames) {
  if (!owner->circuit->get_mutex().try_lock()) {
    return p_frames;
  }

  //std::cout << "mixing input" << std::endl;

  //pass input to the circuit
  mix_in(p_rate_scale, p_frames);

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

