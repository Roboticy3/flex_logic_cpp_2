#pragma once

#include "core/object/ref_counted.h"
#include "core/variant/typed_dictionary.h"
#include "core/variant/variant.h"
#include "servers/audio/audio_stream.h"
#include "tap_sim_live_switch.h"
  
class AudioStreamTapSimulator : public AudioStream {
  GDCLASS(AudioStreamTapSimulator, AudioStream);
  friend class AudioStreamTapSimulatorPlayback;

  TapSimLiveSwitch ls_in;
  
  HashMap<tap_label_t,Ref<AudioStream>> input_streams;

  struct playback_tracker_t {
    Ref<AudioStreamPlayback> playback;
    size_t event_count;
  };

  HashMap<tap_label_t,playback_tracker_t> trackers;

protected:
  static void _bind_methods();

public:
  TypedDictionary<tap_label_t, Ref<AudioStream>> get_input_streams() const;
  void set_input_streams(const TypedDictionary<tap_label_t, Ref<AudioStream>> &streams);

  Ref<TapCircuit> get_circuit() const;
  void set_circuit(Ref<TapCircuit> sim);

  PackedInt64Array get_input_pids() const;
  void set_input_pids(const PackedInt64Array &pids);

  //live is set by the playback's play/stop methods
  bool get_live();

  PackedInt64Array get_event_counts() const;

  virtual Ref<AudioStreamPlayback> instantiate_playback() override;
};

class AudioStreamTapSimulatorPlayback : public AudioStreamPlaybackResampled {
  GDCLASS(AudioStreamTapSimulatorPlayback, AudioStreamPlaybackResampled);
  friend class AudioStreamTapSimulator;

  enum {
		MIX_BUFFER_SIZE = 128
	};

  AudioFrame mix_buffer[MIX_BUFFER_SIZE];

  AudioStreamTapSimulator *owner = nullptr;
  tap_time_t current_time = 0;

protected:
  static void _bind_methods();

public:

  //copy AudioStreamSynchronized mix logic, replacing the summing mix with 
  //sending events to the simulator
  int mix_in(float p_rate_scale, int p_frames);

	//read out the simulator contents
	virtual int mix(AudioFrame *p_buffer, float p_rate_scale, int p_frames) override;

	//start adding events at the latest event position of the simulator and set
  //the live switch on our AudioStreamTapSimulator owner to true.
  //note that setting live can fail, in which case an error prints and nothing
  //happens.
	virtual void start(double p_from_pos = 0.0) override;
  //set live to false
	virtual void stop() override;

  //returns whether the stream is currently playing (live switch is on)
	virtual bool is_playing() const override;

	AudioStreamTapSimulatorPlayback() = default;
	~AudioStreamTapSimulatorPlayback() = default;
};