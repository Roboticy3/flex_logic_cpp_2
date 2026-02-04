#pragma once

#include "core/io/resource.h"
#include "core/templates/vector.h"
#include "core/variant/array.h"
#include "core/variant/typed_array.h"

#include "core/variant/variant.h"
#include "labeling.h"
#include "tap_circuit_types.h"
#include "tap_component_type.h"
#include "tap_patch_bay.h"

/**
 * @brief Implement circuit behavior on a TapPatchBay.
 *
 * Responsible for taking a list of relations and turning that into a circuit.
 * Assumes component definitions do not change while it is active, and does not
 * record connections to non-sensitive inputs.
 *
 * The circuit can be updated by adding and removing components, and updating the
 * connections of existing components. This relatively simple interface should cut
 * down on development time relative to previous iterations.
 *
 * The circuit is also responsible for propagating events through this circuit,
 * implementing at least one function to propagate the lowest event and return the
 * current time. This can be used by audio output to run simulations.
 */
class TapNetwork : public Resource {
	GDCLASS(TapNetwork, Resource)

	Ref<TapPatchBay> patch_bay;

	Labeling<Ref<TapComponentType>> component_types;
	static constexpr tap_label_t WIRE_TYPE = static_cast<tap_label_t>(-1);
	Ref<TapComponentType> wire_component_type;

	Labeling<tap_component_t> components;

protected:
	static void _bind_methods();

public:
	tap_label_t get_invalid_label() const;

	void set_patch_bay(Ref<TapPatchBay> patch_bay);
	Ref<TapPatchBay> get_patch_bay() const;

	void set_component_types(TypedArray<TapComponentType> component_types);
	TypedArray<TapComponentType> get_component_types() const;

	void set_wire_type(Ref<TapComponentType> wire_type);
	Ref<TapComponentType> get_wire_type() const;

	/**
	 * @brief Take a set of pins from the editor and ensure they are unique and valid.
	 *
	 * The returned set contains only valid pin labels.
	 * @param pin_labels Array of pin labels to validate
	 * @return Vector of valid pin labels
	 */
	Vector<tap_label_t> validate_pin_labels(PackedInt64Array pin_labels) const;

	/**
	 * @brief Validate an add_component or move_component operation.
	 *
	 * Tries to create a component with given pins and type. This does not affect the current circuit
	 * and produces a component with populated pins if successful, or empty pins otherwise.
	 * @param pin_labels Array of pin labels to validate
	 * @param component_type_index Index of the component type
	 * @return Validated component or empty if validation failed
	 */
	tap_component_t validate_pin_labels_and_type(PackedInt64Array pin_labels, tap_label_t component_type_index) const;

	/**
	 * @brief Add a component on a set of defined pins.
	 *
	 * Returns -1 if pin_labels is the wrong size for the component type.
	 * component_type_index WIRE_TYPE creates an amorphous wire that can accept
	 * any pin count except zero.
	 * @param pin_labels Array of pin labels for the component
	 * @param component_type_index Index of the component type (defaults to WIRE_TYPE)
	 * @return Label of the added component, or -1 if failed
	 */
	tap_label_t add_component(PackedInt64Array pin_labels, tap_label_t component_type_index = WIRE_TYPE);

	/**
	 * @brief Get the type of a component.
	 *
	 * Returns WIRE_TYPE if the component label is not filled.
	 * @param component_label Label of the component
	 * @return Component type or WIRE_TYPE if not found
	 */
	Ref<TapComponentType> get_component_type(tap_label_t component_label);

	/**
	 * @brief Move a component to a new set of defined pins.
	 *
	 * Fails if new_pin_labels is the wrong size for the component type.
	 * @param component_label Label of the component to move
	 * @param new_pin_labels New array of pin labels for the component
	 * @return True if move was successful, false otherwise
	 */
	bool move_component(tap_label_t component_label, PackedInt64Array new_pin_labels);

	/**
	 * @brief Remove a component.
	 *
	 * Returns true if component_label is a valid component.
	 * @param component_label Label of the component to remove
	 * @return True if component was removed, false if not found
	 */
	bool remove_component(tap_label_t component_label);

	/**
	 * @brief Get the internal component representation
	 *
	 * @param component_label Label of the component
	 * @return Internal component representation or empty if not found
	 */
	std::optional<tap_component_t> get_component_internal(tap_label_t component_label) const;

	/**
	 * @brief Clear all components from this network
	 */
	void clear_components();

	PackedInt64Array get_component_connections(tap_label_t component_label) const;

	/**
	 * @brief Get all component connections.
	 *
	 * Array of PackedInt64Array. Each entry corresponds to a component in the
	 * labeling. If the label is empty, the entry has no connections.
	 * @return Array of component connections
	 */
	Array get_all_component_connections() const;
	/**
	 * @brief Get all component types.
	 *
	 * Array of tap_label_t describing the component types in the labeling. If the
	 * component at a label is empty, the entry is -1.
	 * @return Array of component type indices
	 */
	PackedInt64Array get_all_component_types() const;
};