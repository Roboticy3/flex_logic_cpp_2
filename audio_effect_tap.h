#pragma once

#include "core/object/class_db.h"
#include "servers/audio/audio_effect.h"
#include "core/math/audio_frame.h"
#include "core/math/vector2i.h"

#include "tap_patch_bay.h"
#include "tap_circuit_types.h"

/*
Be an instance of the AudioEffectTap, and maintain a reference to an event queue.
*/
class AudioEffectTapInstance : public AudioEffectInstance {
  GDCLASS(AudioEffectTapInstance, AudioEffectInstance);

  friend class AudioEffectTap;

  //parameters taken from effect resource
  Ref<TapPatchBay> circuit;
  tap_frame::bytes_t activation_delta;
  tap_label_t pid;
  int tick_rate = 1024; //number of simulation ticks per audio sample !TODO!
  //make this property accessible.

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

  Ref<TapPatchBay> circuit; //circuit to send events to
  tap_frame::bytes_t activation_delta = 1; //default to full sensitivity
  tap_label_t pid = 0; //default to  the first pin

  protected:
    static void _bind_methods();

  public:
    virtual Ref<AudioEffectInstance> instantiate() override;

    tap_frame::bytes_t get_activation_delta();
    void set_activation_delta(tap_frame::bytes_t new_activation_delta);

    Ref<TapPatchBay> get_circuit();
    void set_circuit(Ref<TapPatchBay> new_circuit);
    
    tap_label_t get_pid();
    void set_pid(tap_label_t new_pid);


    AudioEffectTap();
};