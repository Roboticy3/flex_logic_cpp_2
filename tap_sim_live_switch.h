#pragma once

#include "core/variant/variant.h"

#include "tap_circuit.h"

/**
 * @brief Separate the logic of checking if a TapCircuit can be simulated (or `live`)
 * from the AudioEffectTapOut class, handling things like locking and pin validation.
 */
class TapSimLiveSwitch {
private:
	bool live = false;
	Ref<TapCircuit> simulator;
	PackedInt64Array live_pids;

public:
	bool get_live() const;
	/**
	 * @brief Only allow live to be set to true if `simulator` is valid an all
	 * `live_pids` are valid pins inside that simulator.
	 */
	void set_live(bool new_live);

	Ref<TapCircuit> get_simulator() const;
	/**
	 * @brief Update the simulator and reset `live` and `live_pids`.
	 */
	void set_simulator(Ref<TapCircuit> new_simulator);

	PackedInt64Array get_live_pids() const;
	/**
	 * @brief Set `live_pids` and set `live` to false if it was previously true
	 * and these new pids are invalid on the current simulator.
	 */
	void set_live_pids(PackedInt64Array new_live_pids);

	/**
	 * @brief Try to lock the simulator for audio simulation up to `end_time` in
	 * simulator ticks.
	 * 
	 * @param end_time The end time in simulator ticks to simulate up to. Set to
	 * zero if you only need to write data.
	 * @param p_dst_frames Uninitialized buffer to fill with 0's if the function
	 * fails.
	 * @param p_frame_count Number of frames to fill in `p_dst_frames`.
	 *
	 * @return true if the simulator was successfully locked under mutex, and 
	 * it contains enough data to analyze up until `end_time`.
	 */
	bool try_lock(tap_time_t end_time, AudioFrame *p_dst_frames, int p_frame_count);

	bool lock() const;
	bool unlock() const;

	/**
	 * @brief Unlock the simulator.
	 *
	 * @return true if the simulator was successfully unlocked, false otherwise.
	 */
	bool unlock();
};