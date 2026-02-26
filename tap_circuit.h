#pragma once

#include <mutex>

#include "core/object/ref_counted.h"

#include "tap_network.h"
#include "tap_patch_bay.h"

/**
 * @brief Aggregate a TapNetwork and TapPatchBay to a full circuit.
 *
 * Resolve state manually or by attaching to an audio process.
 */
class TapCircuit : public Resource {
	GDCLASS(TapCircuit, Resource);

	Ref<TapNetwork> network;
	/// @brief Currently, network composes patch bay, but don't want to rely on that
	Ref<TapPatchBay> patch_bay;

	// Mutex used by audio effects for thread-safe access to the simulator.
	mutable std::recursive_mutex mutex;

	int tick_rate = 1024;
	tap_time_t latest_event_time = 0;
	
	//should be good enough to check this instead of the values of patch_bay and
	//network
	bool instantiated = false;

protected:
	static void _bind_methods();

public:
	Ref<TapNetwork> get_network() const;
	void set_network(Ref<TapNetwork> new_network);

	Ref<TapPatchBay> get_patch_bay() const;
	void set_patch_bay(Ref<TapPatchBay> new_patch_bay);

	int get_tick_rate() const;
	void set_tick_rate(int new_tick_rate);

	tap_time_t get_latest_event_time() const;
	size_t get_event_count() const;

	/**
	 * @brief Clear all elements of the patch bay and network in this simulator.
	 *
	 * Settings like `tick_rate` are not cleared. Component types are not
	 * cleared.
	 */
	void clear();

	/**
	 * @brief Process an event with a priority queue as the source.
	 *
	 * @warning For batch processing only. The circuit must be locked before 
	 * calling this function.
	 *
	 * Pops the top event off of the queue.
	 * @param queue The priority queue to process events from
	 */
	void process_once_internal(tap_queue_t &queue);

	/**
	 * @brief Process an event with the TapCircuit's configured patch bay as the source.
	 *
	 * Uses the patch bay as the source for a queue, and thus the next event.
	 * Since this does not take any internal types as an argument, it can be exposed to the editor.
	 *
	 * @warning For batch processing only. The circuit must be locked before 
	 * calling this function.
	 */
	void process_once();

	/**
	 * @brief Proper simulation function.
	 *
	 * As opposed to the traditional timestep, pass a total time target.
	 *
	 * @warning For batch processing only. The circuit must be locked before 
	 * calling this function.
	 *
	 * @param end_time The target time to simulate to
	 * @return The number of events processed
	 */
	int process_to(tap_time_t end_time);

	/**
	 * @brief Push an event and update the internal latest_event_time value.
	 *
	 * Users can read the latest_event_time to check if there are enough events to 
	 * simulate to a certain time.
	 *
	 * @warning For batch processing only. The circuit must be locked before 
	 * calling this function.
	 *
	 * @param time The time of the event
	 * @param state The audio frame state
	 * @param pid The process label ID
	 */
	void push_event(tap_time_t time, AudioFrame state, tap_label_t pid);

	/**
	 * @brief Mutex getter so Audio processes can make their own locks for batch 
	 * calls. Intended for audio processing.
	 */
	std::recursive_mutex &get_mutex() const;

	/**
	 * @brief Automatically set up a simulator with a minimally configured network + patch bay.
	 *
	 * Comes with:
	 *  - Populated network and patch bay
	 *  - Wire type for added components to default to
	 */
	void instantiate();

	bool is_instantiated() const;

	TapCircuit();
};