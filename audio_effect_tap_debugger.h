
#include "core/object/class_db.h"
#include "core/templates/ring_buffer.h"
#include "core/variant/typed_array.h"
#include "core/variant/typed_dictionary.h"
#include "servers/audio/audio_effect.h"

#include "tap_circuit_types.h"
#include "tap_sim.h"
#include "tap_sim_live_switch.h"
#include "audio_effect_tap_out.h"

/**
 * @brief A statistical snapshot of some amount of simulation time in an
 * AudioEffectTapDebuggerInstance, recovered from AudioEffectTapDebugger using
 * `AudioEffectTapDebugger:get_stats`.
 */
class AudioEffectTapDebuggerQuery : public RefCounted {
  GDCLASS(AudioEffectTapDebuggerQuery, RefCounted)

  PackedVector2Array means;
  PackedVector2Array stddevs;
  PackedInt64Array pids;
  tap_time_t start_time;
  tap_time_t snapshot_end;

  protected:
    static void _bind_methods();

  public:
    PackedVector2Array get_means() const;
    PackedVector2Array get_stddevs() const;
    PackedInt64Array get_pids() const;
    tap_time_t get_start_time() const;
    tap_time_t get_end_time() const;
};

class AudioEffectTapDebuggerInstance : public AudioEffectInstance {
  GDCLASS(AudioEffectTapDebuggerInstance, AudioEffectInstance)

  friend class AudioEffectTapDebugger;

  AudioEffectTapDebugger *effect;

  tap_time_t total_time;

  public:
    virtual void process(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) override;
    virtual bool process_silence() const override;

    /**
     * @brief `process` executes this if `effect->get_live()` is true.
     */
    void process_live(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count);
};

/**
 * @brief Spawn an AudioEffectInstance that analyzes a TapSim on the audio 
 * process schedule. This is a base class that only runs basic statistics on pin
 * states, but the class can be extended to measure performance and accuracy.
 * 
 * Each audio process only results in one state sample, since batching prevents
 * individual sample analysis from being live relative to an AudioEffectTapOut
 * or similar.
 */
class AudioEffectTapDebugger : public AudioEffect {
  GDCLASS(AudioEffectTapDebugger, AudioEffect)

  friend class AudioEffectTapDebuggerInstance;

  //if nullptr, get_stats should return empty
  AudioEffectTapDebuggerInstance *instance;

  //control the circuit, the monitor pids (`ls.get/set_live_pids()`), and the
  //`live` state- whether or not the instance should be simulating.
  TapSimLiveSwitch ls;

  //circular buffer of audio samples
  RingBuffer<AudioFrame> samples; 

  protected:
    static void _bind_methods();

  public:
    Ref<TapSim> get_simulator() const;
    void set_simulator(Ref<TapSim> new_simulator);

    PackedInt64Array get_monitor_pids();
    void set_monitor_pids(PackedInt64Array new_monitor_pids);

    bool get_live() const;
    void set_live(bool new_live);

    TypedArray<PackedVector2Array> get_samples() const;
    AudioEffectTapDebuggerQuery get_stats() const;
};