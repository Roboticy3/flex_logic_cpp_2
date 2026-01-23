#pragma once

#include "core/object/class_db.h"
#include "servers/audio/audio_effect.h"
#include "core/math/audio_frame.h"
#include "core/math/vector2i.h"
#include "core/templates/ring_buffer.h"

#include "tap_circuit_types.h"
#include "tap_sim.h"
#include "tap_sim_live_switch.h"

class AudioEffectTapOutInstance : public AudioEffectInstance {
  GDCLASS(AudioEffectTapOutInstance, AudioEffectInstance)

  friend class AudioEffectTapOut;

  AudioEffectTapOut *effect;

  tap_time_t total_time = 0;
  
  public:
    virtual void process(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) override;
    virtual bool process_silence() const override;

    void process_live(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count);
};

/**
 * @brief Spawn an AudioEffectInstance that solves the circuit on its audio
 * process schedule. If AudioEffectTapIn's are also using the same `simulator`, 
 * they should synchronize and produce a synthesizer.
 * 
 * @param simulator The simulator being processed by this effect.
 * @param output_pids The output pins of `simulator` that should be audible to the
 *  user.
 * @param live If true, the simulator is being processed.
 * @param sample_skip The number of samples between simulation process calls.
 */
class AudioEffectTapOut : public AudioEffect {
  GDCLASS(AudioEffectTapOut, AudioEffect)

  friend class AudioEffectTapOutInstance;

  TapSimLiveSwitch ls;

  int sample_skip = 2;

  protected:
    static void _bind_methods();
  
  public:
    virtual Ref<AudioEffectInstance> instantiate() override;

    /**
     * @brief Find if any `inputs` are connected to param `output_pids` in the
     * `simulator`. Inputs should be all pins actively recieving events from 
     * outside sources. If there is a connection, it might be smart to start
     * solving the simulator using `set_live`.
     */
    bool any_input_connected(PackedInt64Array input_pids);

    Ref<TapSim> get_simulator() const;
    void set_simulator(Ref<TapSim> new_simulator);

    PackedInt64Array get_output_pids() const;
    void set_output_pids(PackedInt64Array new_output_pids);

    bool get_live() const;
    void set_live(bool new_live);

    int get_sample_skip() const;
    void set_sample_skip(int new_sample_skip);

};