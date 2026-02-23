#include <mutex>

#include "core/variant/variant.h"

#include "tap_circuit.h"
#include "tap_sim_live_switch.h"

Ref<TapCircuit> TapSimLiveSwitch::get_simulator() const {
	return simulator;
}
void TapSimLiveSwitch::set_simulator(Ref<TapCircuit> new_simulator) {
	simulator = new_simulator;

	//re-validate the live state
	set_live(get_live());
}

PackedInt64Array TapSimLiveSwitch::get_live_pids() const {
	return live_pids;
}
void TapSimLiveSwitch::set_live_pids(PackedInt64Array new_live_pids) {
	live_pids = new_live_pids;

	//revalidate the simulation
	set_live(get_live());
}

bool TapSimLiveSwitch::get_live() const {
	return live;
}

void TapSimLiveSwitch::set_live(bool new_live) {
	//validate pins before going live
	//audio effects play in the editor, so pre-circuit construction effects will crash if I don't check here :(
	if (new_live) {
		if (simulator.is_null() || simulator->get_patch_bay().is_null()) {
			ERR_PRINT(String("TapSimLiveSwitch::set_live() simulator does not have an instantiated patch bay, cannot go live. Try setting live to true at runtime, after constructing a circuit."));
			live = false;
			return;
		}

		if (live_pids.is_empty()) {
			ERR_PRINT("output pids TapLiveSwitch::get_live_pids() are empty, cannot go live.");
			live = false;
			return;
		}

		for (auto pid : live_pids) {
			if (!simulator->get_patch_bay()->has_pin(pid)) {
				ERR_PRINT(String("output pid ") + itos(pid) + " does not exist yet in AudioEffectTapOut::ls.get_simulator(), cannot go live. Try setting live to true at runtime, after constructing a circuit.");
				live = false;
				return;
			}
		}
	}

	live = new_live;
}

static void fill_audio_buffer(AudioFrame *p_dst_frames, int p_frame_count) {
	for (int i = 0; i < p_frame_count; i++) {
		p_dst_frames[i] = AudioFrame(0.0f, 0.0f);
	}
}

bool TapSimLiveSwitch::try_lock(tap_time_t end_time, AudioFrame *p_dst_frames, int p_frame_count) {
	//guarantee simulator exists and should be live
	if (!live) {
		fill_audio_buffer(p_dst_frames, p_frame_count);
		return false;
	}

	//make simulator safe to read
	std::recursive_mutex &mutex = simulator->get_mutex();
	if (!mutex.try_lock()) {
		fill_audio_buffer(p_dst_frames, p_frame_count);
		return false;
	}

	if (end_time == 0) {
		return true;
	}

	//read the latest event time and compare with end_time
	tap_time_t latest_event_time = simulator->get_latest_event_time();
	
	if (latest_event_time < end_time) {
		mutex.unlock();
		fill_audio_buffer(p_dst_frames, p_frame_count);
		return false;
	}

	return true;
}

bool TapSimLiveSwitch::lock() const {
	if (simulator.is_null()) {
		return false;
	}
	simulator->get_mutex().lock();
	return true;
}

bool TapSimLiveSwitch::unlock() const {
	if (simulator.is_null()) {
		return false;
	}
	
	std::recursive_mutex &mutex = simulator->get_mutex();

	mutex.unlock();
	
	return true;
}

bool TapSimLiveSwitch::unlock() {
	if (simulator.is_null()) {
		return false;
	}
	
	std::recursive_mutex &mutex = simulator->get_mutex();
	mutex.unlock();
	
	return true;
}