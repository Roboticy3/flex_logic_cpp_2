#pragma once

#include "core/object/ref_counted.h"
#include "core/variant/variant.h"
#include "scene/resources/audio_stream_wav.h"
#include "servers/audio/audio_stream.h"
#include "tap_sim_live_switch.h"
  
class AudioStreamTapIn : public AudioStreamWAV {
  GDCLASS(AudioStreamTapIn, AudioStreamWAV);
  friend class AudioStreamTapInPlayback;

  TapSimLiveSwitch ls_in;
  Ref<AudioStream> base;

protected:
  static void _bind_methods();

public:
  Ref<AudioStream> get_base() { return base; }
  void set_base(Ref<AudioStream> stream) { base = stream; }

  Ref<TapSim> get_simulator() { return ls_in.get_simulator(); }
  void set_simulator(Ref<TapSim> sim) { ls_in.set_simulator(sim); }

  PackedInt64Array get_input_pids() { return ls_in.get_live_pids(); }
  void set_input_pids(const PackedInt64Array &pids) { ls_in.set_live_pids(pids); }

  //live is set by the playback's play/stop methods
  bool get_live() { return ls_in.get_live(); }

  virtual Ref<AudioStreamPlayback> instantiate_playback() override;
};

class AudioStreamTapInPlayback : public AudioStreamPlaybackResampled {
  GDCLASS(AudioStreamTapInPlayback, AudioStreamPlaybackResampled);
  friend class AudioStreamTapIn;

  Ref<AudioStreamPlaybackWAV> composition_over_inheritance;
  AudioStreamTapIn *owner = nullptr;
  tap_time_t current_time = 0;
  size_t event_count = 0;

protected:
  static void _bind_methods();

public:
  size_t get_event_count() const { return event_count; }

	//read out the simulator contents
	virtual int mix(AudioFrame *p_buffer, float p_rate_scale, int p_frames) override;

	//start adding events at the latest event position of the simulator and set
  //the live switch on our AudioStreamTapIn owner to true.
  //note that setting live can fail, in which case an error prints and nothing
  //happens.
	virtual void start(double p_from_pos = 0.0) override;
  //set live to false
	virtual void stop() override;

  //returns whether the stream is currently playing (live switch is on)
	virtual bool is_playing() const override;

	AudioStreamTapInPlayback() = default;
	~AudioStreamTapInPlayback() = default;
};