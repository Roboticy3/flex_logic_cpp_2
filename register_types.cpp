#include "core/object/class_db.h"

#include "register_types.h"
#include "frame_cache.h"
#include "tap_user.h"
#include "audio_effect_tap.h"

void initialize_flex_logic_cpp_2_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	ClassDB::register_class<FrameCache>();
	ClassDB::register_class<TapUser>();
	ClassDB::register_class<AudioEffectTap>();
	ClassDB::register_class<AudioEffectTapInstance>();
}

void uninitialize_flex_logic_cpp_2_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}
