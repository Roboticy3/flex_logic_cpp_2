#pragma once

#include "core/object/object.h"
#include "core/templates/hash_map.h"
#include "core/variant/typed_array.h"
#include "servers/audio/audio_effect.h"

#include "tap_circuit_types.h"
#include "tap_sim.h"
#include "tap_sim_live_switch.h"

class AudioEffectTapDebugger;

/**
 * @brief A statistical snapshot of some amount of simulation time in an
 * AudioEffectTapDebuggerInstance, recovered from AudioEffectTapDebugger using
 * `AudioEffectTapDebugger:get_stats`.
 */
class AudioEffectTapDebuggerQuery : public RefCounted {
	GDCLASS(AudioEffectTapDebuggerQuery, RefCounted)

	friend class AudioEffectTapDebugger;

	PackedVector2Array means;
	PackedVector2Array stddevs;
	PackedInt64Array pids;
	tap_time_t start_time;
	tap_time_t end_time;

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

	//control the circuit, the monitor pids (`ls.get/set_live_pids()`), and the
	//`live` state- whether or not the instance should be simulating.
	TapSimLiveSwitch ls;

	HashMap<tap_label_t, Vector<AudioFrame>> samples;
	int sample_count = 32;
	tap_time_t samples_start_time;
	tap_time_t samples_end_time;

protected:
	static void _bind_methods();

public:
	Ref<TapSim> get_simulator() const;
	void set_simulator(Ref<TapSim> new_simulator);

	PackedInt64Array get_monitor_pids() const;
	void set_monitor_pids(PackedInt64Array new_monitor_pids);

	bool get_live() const;
	void set_live(bool new_live);

	int get_sample_count() const;
	void set_sample_count(int new_sample_count);

	TypedArray<PackedVector2Array> get_samples() const;
	Ref<AudioEffectTapDebuggerQuery> get_stats() const;

	virtual Ref<AudioEffectInstance> instantiate() override;
};