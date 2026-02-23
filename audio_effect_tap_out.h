#pragma once

#include "core/math/audio_frame.h"
#include "core/templates/local_vector.h"
#include "servers/audio/audio_effect.h"

#include "tap_circuit_types.h"
#include "tap_circuit.h"
#include "tap_sim_live_switch.h"
#include "reference_sim.h"

class AudioEffectTapOut;

/**
 * @brief AudioEffectInstance that solves the circuit on its audio process
 * schedule. If AudioEffectTapIn's are also using the same `simulator`, they
 * should synchronize and produce a synthesizer.
 * 
 * @param effect A reference to the AudioEffectTapOut that spawned this instance
 * @param total_time The total ticks passed in simulation time since the
 * creation of this effect. Simulation will wait until inputs have passed this
 * time.
 * @param phase Unused.
 * @param inputs The last recorded input state right before simulation. Should
 * not change size after construction.
 * @param outputs The outputs of the instance. Should not change size after
 * construction.
 */
class AudioEffectTapOutInstance : public AudioEffectInstance {
	GDCLASS(AudioEffectTapOutInstance, AudioEffectInstance)

	friend class AudioEffectTapOut;

	AudioEffectTapOut *effect;

	tap_time_t total_time = 0;
	double phase = 0.0;

	LocalVector<AudioFrame> inputs;
	LocalVector<AudioFrame> outputs;
	int mix_rate;

	size_t event_count = 0;

protected:
	static void _bind_methods();

public:
	size_t get_event_count() const { return event_count; }

	virtual void process(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) override;
	virtual bool process_silence() const override;

	void process_live(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count);
};

/**
 * @brief Spawn an AudioEffectInstance that solves the circuit on its audio
 * process schedule. If AudioEffectTapIn's are also using the same `simulator`,
 * they should synchronize and produce a synthesizer.
 *
 * @param ls Wrap the simulator and `live` state, validating whether `live` can
 * be set to true.
 * @param sample_skip The number of samples between simulation process calls.
 */
class AudioEffectTapOut : public AudioEffect {
	GDCLASS(AudioEffectTapOut, AudioEffect)

	friend class AudioEffectTapOutInstance;

	TapSimLiveSwitch ls_out;
	TapSimLiveSwitch ls_in;
	Ref<ReferenceSim> reference_sim;

	int sample_skip = 2;

protected:
	static void _bind_methods();

public:
	virtual Ref<AudioEffectInstance> instantiate() override;

	/**
	 * @brief Find if any `inputs` are connected to param `output_pids` in the
	 * `simulator`. Inputs should be all pins actively recieving events from
	 * outside sources.
	 */
	bool any_input_connected();

	Ref<TapCircuit> get_simulator() const;
	void set_simulator(Ref<TapCircuit> new_simulator);

	PackedInt64Array get_output_pids() const;
	void set_output_pids(PackedInt64Array new_output_pids);

	PackedInt64Array get_input_pids() const;
	void set_input_pids(PackedInt64Array new_input_pids);

	Ref<ReferenceSim> get_reference_sim() const;
	void set_reference_sim(Ref<ReferenceSim> new_reference_sim);

	bool get_live() const;
	void set_live(bool new_live);

	int get_sample_skip() const;
	void set_sample_skip(int new_sample_skip);
};