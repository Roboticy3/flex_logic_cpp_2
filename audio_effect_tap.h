#pragma once

#include "core/object/class_db.h"
#include "servers/audio/audio_effect.h"
#include "core/math/audio_frame.h"
#include "core/math/vector2i.h"

#include "circuit_tap.h"

/*
Be an instance of the AudioEffectTap, and maintain a reference to an event queue.
*/
class AudioEffectTapInstance : public AudioEffectInstance {
  GDCLASS(AudioEffectTapInstance, AudioEffectInstance);

  friend class AudioEffectTap;

  //parameters taken from effect resource
  Ref<CircuitTap> circuit;
  tap_frame::bytes_t activation_delta;
  tap_label_t component_id;

  //instance specific state
  tap_frame last_activation;
  tap_time_t total_time = 0;
  
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
  tap_frame::bytes_t activation_delta = 1;
  tap_label_t component_id = -1;

  protected:
    static void _bind_methods();

  public:
    virtual Ref<AudioEffectInstance> instantiate() override;

    tap_frame::bytes_t get_activation_delta();
    void set_activation_delta(tap_frame::bytes_t new_activation_delta);

    Ref<CircuitTap> get_circuit();
    void set_circuit(Ref<CircuitTap> new_circuit);


    AudioEffectTap();
};