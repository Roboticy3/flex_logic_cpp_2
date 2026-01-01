#include "core/object/class_db.h"

#include "register_types.h"
#include "tap_user.h"
#include "audio_effect_tap.h"
#include "tap_component_type.h"

void initialize_flex_logic_cpp_2_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	ClassDB::register_class<TapUser>();
	ClassDB::register_class<TapComponentType>();
	ClassDB::register_class<AudioEffectTap>();
	ClassDB::register_class<AudioEffectTapInstance>();

	TapComponentType::initialize_solver_registry_internal();
}

void uninitialize_flex_logic_cpp_2_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	TapComponentType::uninitialize_solver_registry_internal();
}
