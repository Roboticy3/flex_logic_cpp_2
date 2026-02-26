#pragma once

#include "core/object/object.h"
#include "core/variant/variant.h"
#include "scene/2d/audio_stream_player_2d.h"
#include "servers/audio/audio_stream.h"
#include "tap_sim_live_switch.h"

class AudioStreamTapProbePlayback;

class AudioStreamTapProbe : public AudioStream {
  GDCLASS(AudioStreamTapProbe, AudioStream)
	friend class AudioStreamTapProbePlayback;

	TapSimLiveSwitch ls_in;

  protected:
    static void _bind_methods();

  public:
		Ref<TapCircuit> get_simulator() {return ls_in.get_simulator();}
		void set_simulator(Ref<TapCircuit> new_sim) {ls_in.set_simulator(new_sim);}

		PackedInt64Array get_input_pids() {return ls_in.get_live_pids();}
		void set_input_pids(PackedInt64Array pids) {ls_in.set_live_pids(pids);}

		bool get_live() {return ls_in.get_live();}

    virtual Ref<AudioStreamPlayback> instantiate_playback() override;
    
};

class AudioStreamTapProbePlayback : public AudioStreamPlaybackResampled {
	GDCLASS(AudioStreamTapProbePlayback, AudioStreamPlaybackResampled);
	friend class AudioStreamTapProbe;

  AudioStreamTapProbe* stream = nullptr;
	
  double phase = 0.0;
  double frequency = 440.0;
  size_t mix_rate = 44100;

protected:
  static void _bind_methods();

	virtual float get_stream_sampling_rate() override;
	virtual double get_playback_position() const override;

public:
	//read out the simulator contents
	virtual int mix(AudioFrame *p_buffer, float p_rate_scale, int p_frames) override;

	//try to set the live state of the live switch.
	virtual void start(double p_from_pos = 0.0) override;
	virtual void stop() override;

	virtual bool is_playing() const override;

	virtual int get_loop_count() const override; //times it looped

	virtual void seek(double p_time) override;

	virtual void tag_used_streams() override;

	AudioStreamTapProbePlayback();
	~AudioStreamTapProbePlayback();
};