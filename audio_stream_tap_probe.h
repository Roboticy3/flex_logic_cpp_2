
#include "core/object/object.h"
#include "scene/2d/audio_stream_player_2d.h"
#include "servers/audio/audio_stream.h"

class AudioStreamTapProbe : public AudioStream {
  GDCLASS(AudioStreamTapProbe, AudioStream)

  protected:
    static void _bind_methods();

  public:
    virtual Ref<AudioStreamPlayback> instantiate_playback() override;
    
};

class AudioStreamTapProbePlayback : public AudioStreamPlaybackResampled {
	GDCLASS(AudioStreamTapProbePlayback, AudioStreamPlaybackResampled);
	friend class AudioStreamTapProbe;

	bool active = false;

  AudioStreamTapProbe* stream = nullptr;
	
  double phase = 0.0;
  double frequency = 440.0;
  size_t mix_rate = 44100;

protected:
  static void _bind_methods();

	virtual float get_stream_sampling_rate() override;
	virtual double get_playback_position() const override;

public:
	virtual int mix(AudioFrame *p_buffer, float p_rate_scale, int p_frames) override;

	virtual void start(double p_from_pos = 0.0) override;
	virtual void stop() override;
	virtual bool is_playing() const override;

	virtual int get_loop_count() const override; //times it looped

	virtual void seek(double p_time) override;

	virtual void tag_used_streams() override;

	AudioStreamTapProbePlayback();
	~AudioStreamTapProbePlayback();
};