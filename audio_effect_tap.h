#pragma once

#include "core/object/class_db.h"
#include "servers/audio/audio_effect.h"
#include "core/math/audio_frame.h"

#include "circuit_tap.h"


/*
Be an instance of the AudioEffectTap, and maintain a reference to an event queue.
*/
class AudioEffectTapInstance : public AudioEffectInstance {
  GDCLASS(AudioEffectTapInstance, AudioEffectInstance);

  friend class AudioEffectTap;

  Ref<CircuitTap> circuit;
  
  public:
    virtual void process(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) override;
    virtual bool process_silence() const override;
};

/*
Convert an audio bus into events based on an activation delta
*/
class AudioEffectTap : public AudioEffect {
  GDCLASS(AudioEffectTap, AudioEffect);

  friend class AudioEffectTapInstance;

  Ref<CircuitTap> circuit;
  float activation_delta = 0.0;

  protected:
    static void _bind_methods();

  public:
    virtual Ref<AudioEffectInstance> instantiate() override;

    float get_activation_delta();
    void set_activation_delta(float new_activation_delta);

    AudioEffectTap();
};