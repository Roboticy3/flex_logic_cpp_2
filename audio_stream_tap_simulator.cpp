#include "audio_stream_tap_simulator.h"
#include "core/object/object.h"
#include "core/variant/variant.h"
#include "servers/audio/audio_stream.h"
#include "tap_circuit_types.h"
#include <iostream>

void AudioStreamTapSimulator::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_input_streams"), &AudioStreamTapSimulator::get_input_streams);
  ClassDB::bind_method(D_METHOD("set_input_streams", "streams"), &AudioStreamTapSimulator::set_input_streams);
  ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "input_streams", PROPERTY_HINT_DICTIONARY_TYPE, "int:AudioStream", PROPERTY_USAGE_EDITOR), "set_input_streams", "get_input_streams");

  ClassDB::bind_method(D_METHOD("get_circuit"), &AudioStreamTapSimulator::get_circuit);
  ClassDB::bind_method(D_METHOD("set_circuit", "circuit"), &AudioStreamTapSimulator::set_circuit);
  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "circuit", PROPERTY_HINT_RESOURCE_TYPE, "TapCircuit"), "set_circuit", "get_circuit");

  ClassDB::bind_method(D_METHOD("get_input_pids"), &AudioStreamTapSimulator::get_input_pids);
  ClassDB::bind_method(D_METHOD("set_input_pids", "pids"), &AudioStreamTapSimulator::set_input_pids);
  ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT64_ARRAY, "input_pids"), "set_input_pids", "get_input_pids");

  ClassDB::bind_method(D_METHOD("get_live"), &AudioStreamTapSimulator::get_live);
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

Ref<TapCircuit> AudioStreamTapSimulator::get_circuit() const {
  return ls_in.get_simulator();
}

void AudioStreamTapSimulator::set_circuit(Ref<TapCircuit> sim) {
  ls_in.set_simulator(sim);
}

PackedInt64Array AudioStreamTapSimulator::get_input_pids() const {
  return ls_in.get_live_pids();
}

void AudioStreamTapSimulator::set_input_pids(const PackedInt64Array &pids) {
  ls_in.set_live_pids(pids);
}

bool AudioStreamTapSimulator::get_live() {
  return ls_in.get_live();
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
  if (!ls_in.lock()) {
    return PackedInt64Array();
  }
  
  PackedInt64Array arr;
  for (auto kv: trackers) {
    arr.push_back(kv.value.event_count);
  }
  ls_in.unlock();
  return arr;
}

void AudioStreamTapSimulatorPlayback::_bind_methods() {};

int AudioStreamTapSimulatorPlayback::mix_in(float p_rate_scale, int p_frames) {
	if (!owner->get_live()) {
		return 0;
	}

  Ref<TapCircuit> simulator = owner->ls_in.get_simulator();

	int todo = p_frames;

	bool any_active = false;
	while (todo) {
		int to_mix = MIN(todo, MIX_BUFFER_SIZE);

		bool first = true;
		for (auto &kv : owner->trackers) {
      tap_label_t label = kv.key;
			auto &tracker = kv.value;
			if (tracker.playback.is_valid() && tracker.playback->is_playing()) {
				
        //TODO: volume controls for input streams

				if (first) {
					any_active = true;
				}

        tracker.playback->mix(mix_buffer, p_rate_scale, to_mix);
        for (int j = 0; j < to_mix; j++) {  
          //input circuit events here.
          tap_time_t time = current_time + (j * p_rate_scale) * simulator->get_tick_rate();
          simulator->push_event(time, mix_buffer[j], label);
        }

        tracker.event_count += to_mix;

        std::cout << "\tincrementing " << tracker.playback.ptr() << " to " << tracker.event_count << std::endl;
			}
		}
		todo -= to_mix;
	}

	if (!any_active) {
		owner->ls_in.set_live(false);
	}
	return p_frames;
}

int AudioStreamTapSimulatorPlayback::mix(AudioFrame *p_buffer, float p_rate_scale, int p_frames) {

  Ref<TapCircuit> simulator = owner->ls_in.get_simulator();

  if (!owner->ls_in.try_lock(0, p_buffer, p_frames)) {
    return p_frames;
  }

  std::cout << "mixing input" << std::endl;

  //pass input to the circuit
  mix_in(p_rate_scale, p_frames);

  PackedInt64Array input_pids = owner->get_input_pids();

  for (int i = 0; i < p_frames; i++) {
    tap_time_t frame_time = current_time + (i * p_rate_scale) * simulator->get_tick_rate();
    for (tap_label_t pid : input_pids) {
      simulator->push_event(frame_time, p_buffer[i], pid);
    }
  }

  current_time += (p_frames * p_rate_scale) * simulator->get_tick_rate();

  owner->ls_in.unlock();
  
  return p_frames;
}

void AudioStreamTapSimulatorPlayback::start(double p_from_pos) {
  owner->ls_in.set_live(true);
  if (owner->ls_in.get_live()) {
    current_time = owner->ls_in.get_simulator()->get_latest_event_time();

    for (auto kv : owner->trackers) {
      kv.value.event_count = 0;
      kv.value.playback->start(p_from_pos);
    }
  }
}

void AudioStreamTapSimulatorPlayback::stop() {
  owner->ls_in.set_live(false);

  if (owner->ls_in.get_live()) {
    for (auto kv : owner->trackers) {
      kv.value.playback->stop();
    }
  }
}

bool AudioStreamTapSimulatorPlayback::is_playing() const {
  return owner->get_live();
}

