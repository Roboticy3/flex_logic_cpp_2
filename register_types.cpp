#include "core/object/class_db.h"

#include "register_types.h"
#include "tap_component_type.h"
#include "tap_patch_bay.h"
#include "tap_network.h"
#include "tap_sim.h"
#include "tap_sim_generator.h"
#include "audio_effect_tap.h"

void initialize_flex_logic_cpp_2_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	//initialize TapComponentType before registry so the drop down menu loads
	//correctly.
	TapComponentType::initialize_solver_registry_internal();

	ClassDB::register_class<TapComponentType>();
	
	ClassDB::register_class<TapPatchBay>();
	ClassDB::register_class<TapNetwork>();
	ClassDB::register_class<TapSim>();

	ClassDB::register_class<TapSimGenerator>();
	ClassDB::register_class<TapSimGeneratorPlayback>();
	ClassDB::register_class<AudioEffectTap>();
	ClassDB::register_class<AudioEffectTapInstance>();
}

void uninitialize_flex_logic_cpp_2_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	TapComponentType::uninitialize_solver_registry_internal();
}
