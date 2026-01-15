#pragma once

#include "core/object/class_db.h"
#include "servers/audio/audio_effect.h"
#include "core/math/audio_frame.h"
#include "core/math/vector2i.h"

#include "tap_circuit_types.h"
#include "tap_sim.h"

/*
Be an instance of the AudioEffectTap, and maintain a reference to an event queue.
*/
class AudioEffectTapInstance : public AudioEffectInstance {
  GDCLASS(AudioEffectTapInstance, AudioEffectInstance);

  friend class AudioEffectTap;

  //parameters taken from effect resource
  AudioEffectTap *effect;

  //instance specific state
  tap_frame last_activation;
  tap_time_t total_time = 0;
  
  public:
    // Drive the effect's tap_sim
    virtual void process(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) override;
    virtual bool process_silence() const override;

    void process_line_in(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count);
};

/*
Spawn an AudioEffectInstance to drive a TapSim via the AudioEffectInstance
process method.
*/
class AudioEffectTap : public AudioEffect {
  GDCLASS(AudioEffectTap, AudioEffect);

  friend class AudioEffectTapInstance;

  Ref<TapSim> simulator; //circuit to send events to
  tap_frame::bytes_t activation_delta = 1; //default to full sensitivity
  tap_label_t pid = 0; //default to  the first pin

  int tick_rate = 1024;
  bool line_in = true;
  bool line_out = true;

  protected:
    static void _bind_methods();

  public:
    virtual Ref<AudioEffectInstance> instantiate() override;

    Ref<TapSim> get_simulator() const;
    void set_simulator(Ref<TapSim> new_simulator);

    tap_frame::bytes_t get_activation_delta() const;
    void set_activation_delta(tap_frame::bytes_t new_activation_delta);
    
    tap_label_t get_pid() const;
    void set_pid(tap_label_t new_pid);

    int get_tick_rate() const;
    void set_tick_rate(int new_tick_rate);

    bool get_line_in() const;
    void set_line_in(bool new_line_in);

    bool get_line_out() const;
    void set_line_out(bool new_line_out);

    AudioEffectTap();
};