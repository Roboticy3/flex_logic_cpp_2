
#include "core/object/class_db.h"
#include "core/templates/vector.h"

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
  
  Vector<tap_component_type_t> components_types;
  HashMap<tap_label_t, tap_component_t> components;

  protected:
    static void _bind_methods();
};