#pragma once

#include <mutex>

#include "core/typedefs.h"
#include "core/object/object.h"
#include "core/variant/variant.h"
#include "scene/2d/audio_stream_player_2d.h"
#include "servers/audio/audio_stream.h"

class AudioStreamPrimitivePlayback;

class AudioStreamPrimitive : public AudioStream {
  GDCLASS(AudioStreamPrimitive, AudioStream)
	friend class AudioStreamPrimitivePlayback;

  float frequency = 440.0f;
  float amplitude = 1.0f;

  float sin = 1.0f;
  float tri = 0.0f;
  float sqr = 0.0f;
  float saw = 0.0f;

  using mutex_t = std::mutex;
  mutable mutex_t audio_playback_mutex;

  protected:
    static void _bind_methods();

  public:
    float get_frequency() const;
    void set_frequency(float p_frequency);

    float get_amplitude() const;
    void set_amplitude(float p_amplitude);

    float get_sin() const;
    void set_sin(float p_sin);
    
    float get_tri() const;
    void set_tri(float p_tri);
    
    float get_sqr() const;
    void set_sqr(float p_sqr);
    
    float get_saw() const;
    void set_saw(float p_saw);

    void normalize();

    Ref<AudioStreamPlayback> instantiate_playback() override;
    
};

class AudioStreamPrimitivePlayback : public AudioStreamPlaybackResampled {
	GDCLASS(AudioStreamPrimitivePlayback, AudioStreamPlaybackResampled);
	friend class AudioStreamPrimitive;

  AudioStreamPrimitive* stream = nullptr;
	
  double phase = 0.0;
  double frequency = 440.0;
  size_t mix_rate = 44100;

  bool playing = false;

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

	AudioStreamPrimitivePlayback();
	~AudioStreamPrimitivePlayback();
};