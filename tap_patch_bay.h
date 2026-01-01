#pragma once

#include <optional>

#include "core/object/class_db.h"
#include "core/io/resource.h"
#include "core/variant/variant.h"
#include "core/variant/typed_dictionary.h"
#include "core/templates/hash_map.h"

#include "circuit.h"
#include "tap_circuit_types.h"


/*
Store templates for circuit primitives that can be attached to an audio tap. Not
responsible for event propogation, only maintaining pins and event queue.

Also used to expose basic event operations to the editor. Unregistered methods
will be named *_internal.

Queue is a protected member for easy access during simulation.
*/
class TapPatchBay : public Resource {
  GDCLASS(TapPatchBay, Resource)

  //number of samples made on the last event pass
  int samples = 0;

  protected:
    static void _bind_methods();

    //event queue
    tap_queue_t queue;

    //pin mapping
    HashMap<tap_label_t, tap_pin_t> pins;

  public:

    static constexpr Vector2i STATE_MISSING = Vector2i(2, 2);
    inline Vector2i get_state_missing() {
      return STATE_MISSING;
    }

    int get_event_count();
    //if no events are available, returns (2,2)
    Vector2i pop_event();
    std::optional<tap_event_t> pop_event_internal();
    void push_event_internal(tap_event_t event);

    int get_sample_count();
    void set_sample_count_internal(int new_samples);

    void add_pin(tap_label_t label, Vector2i initial_state);
    void add_pin_with_frame(tap_label_t label, Vector2 initial_state);
    void remove_pin(tap_label_t label);

    Vector2i get_pin_state(tap_label_t label);
    TypedDictionary<tap_label_t, Vector2i> all_pin_states();
    void set_pin_state(tap_label_t label, Vector2i new_state);
    void set_pin_state_with_frame(tap_label_t label, Vector2 new_state);
    
    TapPatchBay() = default;
};