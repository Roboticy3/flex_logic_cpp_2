#include "core/object/class_db.h"

#include "audio_effect_tap_debugger.h"
#include "audio_effect_tap_in.h"
#include "audio_effect_tap_out.h"
#include "register_types.h"
#include "tap_component_type.h"
#include "tap_network.h"
#include "tap_patch_bay.h"
#include "tap_sim.h"

void initialize_flex_logic_cpp_2_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	//initialize TapComponentType before registry so the drop down menu loads
	//correctly.
	TapComponentType::initialize_solver_registry_internal();

	ClassDB::register_class<TapFrame>();
	ClassDB::register_class<TapComponentType>();

	ClassDB::register_class<TapPatchBay>();
	ClassDB::register_class<TapNetwork>();
	ClassDB::register_class<TapSim>();

	ClassDB::register_class<AudioEffectTapIn>();
	ClassDB::register_class<AudioEffectTapInInstance>();
	ClassDB::register_class<AudioEffectTapOut>();
	ClassDB::register_class<AudioEffectTapOutInstance>();

	ClassDB::register_class<AudioEffectTapDebugger>();
	ClassDB::register_class<AudioEffectTapDebuggerInstance>();
	ClassDB::register_class<AudioEffectTapDebuggerQuery>();
}

void uninitialize_flex_logic_cpp_2_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	TapComponentType::uninitialize_solver_registry_internal();
}
