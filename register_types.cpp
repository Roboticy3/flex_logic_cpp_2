#include "core/object/class_db.h"

#include "register_types.h"
#include "tap_component_type.h"
#include "tap_network.h"
#include "tap_patch_bay.h"
#include "tap_circuit.h"
#include "reference_sim.h"
#include "audio_stream_tap_simulator.h"
#include "audio_stream_primitive.h"

void initialize_flex_logic_cpp_2_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	//initialize TapComponentType before registry so the drop down menu loads
	//correctly.
	TapComponentType::initialize_solver_registry_internal();
	ReferenceSim::initialize_reference_registry_internal();

	ClassDB::register_class<TapFrame>();
	ClassDB::register_class<TapComponentType>();

	ClassDB::register_class<TapPatchBay>();
	ClassDB::register_class<TapNetwork>();
	ClassDB::register_class<TapCircuit>();
	ClassDB::register_class<ReferenceSim>();

	ClassDB::register_class<AudioStreamTapSimulator>();
	ClassDB::register_class<AudioStreamTapSimulatorPlayback>();
	ClassDB::register_class<AudioStreamPrimitive>();
}

void uninitialize_flex_logic_cpp_2_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	TapComponentType::uninitialize_solver_registry_internal();
	ReferenceSim::uninitialize_reference_registry_internal();
}
