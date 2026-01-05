
#include "core/object/class_db.h"
#include "core/templates/vector.h"

#include "labeling.h"
#include "tap_circuit_types.h"

/*
Implement circuit behavior on a TapPatchBay

Responsible for taking a list of relations and turning that into a circuit. 
Assumes component definitions do not change while it is active, and does not
record connections to non-sensitive inputs.

The circuit can be updated by adding and removing components, and updating the
connections of existing components. This relatively simple interface should cut
down on development time relative to previous iterations.

The circuit is also responsible for propogating events through this circuit, 
implementing at least one function to propogate the lowest event and return the
current time. This can be used by audio output to run simulations.
*/
class TapNetwork : public Resource {
  GDCLASS(TapNetwork, Resource)
  
  Vector<Ref<TapComponentType>> components_types;
  static constexpr tap_label_t WIRE_TYPE = static_cast<tap_label_t>(-1);
  
  Labeling<tap_component_t> components;

  protected:
    static void _bind_methods();
  
  public:
    tap_label_t get_invalid_label() const;

    void set_component_types(Vector<Ref<TapComponentType>> component_types);
    Vector<Ref<TapComponentType>> get_component_types() const;

    /*
    Add a component on a set of defined pins. Returns -1 if `pin_labels` is the
    wrong size for the component type.

    `component_type_index` WIRE_TYPE creates an amorphous wire that can accept
    any pin count except zero.
    */
    tap_label_t add_component(Vector<tap_label_t> pin_labels, tap_label_t component_type_index=WIRE_TYPE);

    /*
    Move a component to a new set of defined pins. Fails if `new_pin_labels` is
    the wrong size for the component type.
    */
    bool move_component(tap_label_t component_label, Vector<tap_label_t> new_pin_labels);

    /*
    Remove a component. Returns true if `component_label` is a valid component.
    */
    bool remove_component(tap_label_t component_label);
};