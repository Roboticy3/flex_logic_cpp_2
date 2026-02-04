#include "core/variant/variant.h"

#include "tap_sim.h"
#include "tap_sim_live_switch.h"

Ref<TapSim> TapSimLiveSwitch::get_simulator() const {
	return simulator;
}
void TapSimLiveSwitch::set_simulator(Ref<TapSim> new_simulator) {
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