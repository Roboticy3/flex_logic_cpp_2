#pragma once

#include "core/math/audio_frame.h"
#include "servers/audio/audio_effect.h"

#include "tap_circuit_types.h"
#include "tap_sim.h"

/**
 * @brief Reference an AudioEffectTapIn and execute on audio using its
 *  parameters.
 *
 * @param effect Reference to AudioEffectTapIn
 * @param last_activation Last event passed to `effect->simulator`
 * @param total_time Total time in simulation ticks since last `process` call.
 *  For frame `i` in a given process call, the timing on the event pushed to the
 *  circuit is `total_time + i * effect->simulator->tick_rate`.
 */
class AudioEffectTapInInstance : public AudioEffectInstance {
	GDCLASS(AudioEffectTapInInstance, AudioEffectInstance);

	friend class AudioEffectTapIn;

	//parameters taken from effect resource
	AudioEffectTapIn *effect;

	//instance specific state
	AudioFrame last_activation;
	tap_time_t total_time = 0;

public:
	virtual void process(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) override;
	virtual bool process_silence() const override;

	/**
	 * @brief If `effect->line_in` is true, `process` executes this method to
	 * push a package of audio frames onto `effect->simulator`.
	 */
	void process_line_in(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count);
};

/**
 * @brief Referencing a TapSim and a single pin id for input, create an
 * AudioEffectInstance to convert audio samples into events and push them into
 * `simulator` with simulation time sample# * `simulator->tick_rate`.
 *
 * @param simulator The target TapSim events are fed into
 * @param pid The target pin in `simulator` events are fed into
 * @param activation_delta The difference between samples in amplitude
 *  constituting an "event". When set to 1 or lower, any change in audio level
 *  is an event.
 * @param line_in When false, samples are *not* passed to the simulator, as
 *  if `activation_delta` were infinitely large.
 * @param line_out When true, samples caught by this effect are passed along
 *  the effect chain, when false, audio is silenced on the bus holding this
 *  effect.
 */
class AudioEffectTapIn : public AudioEffect {
	GDCLASS(AudioEffectTapIn, AudioEffect);

	friend class AudioEffectTapInInstance;

	Ref<TapSim> simulator;
	tap_sample_t activation_delta = 0.0;
	tap_label_t pid = 0;

	bool line_in = true;
	bool line_out = false;

	int sample_skip = 2;

protected:
	static void _bind_methods();

public:
	virtual Ref<AudioEffectInstance> instantiate() override;

	Ref<TapSim> get_simulator() const;
	void set_simulator(Ref<TapSim> new_simulator);

	tap_sample_t get_activation_delta() const;
	void set_activation_delta(tap_sample_t new_activation_delta);

	tap_label_t get_pid() const;
	void set_pid(tap_label_t new_pid);

	bool get_line_in() const;
	void set_line_in(bool new_line_in);

	bool get_line_out() const;
	void set_line_out(bool new_line_out);

	int get_sample_skip() const;
	void set_sample_skip(int new_sample_skip);

	AudioEffectTapIn();
};