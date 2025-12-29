#pragma once

#include <optional>

#include "core/object/class_db.h"
#include "core/io/resource.h"
#include "core/variant/variant.h"
#include "core/templates/hash_map.h"

#include "circuit.h"
#include "tap_circuit_types.h"


/*
Store templates for circuit primitives that can be attached to an audio tap
*/
class TapUser : public Resource {
  GDCLASS(TapUser, Resource)

  //number of samples made on the last event pass
  int samples = 0;

  protected:
    static void _bind_methods();

    //event queue
    tap_queue_t queue;

  public:

    int get_event_count();
    Vector2i pop_event();
    std::optional<tap_event_t> pop_event_internal();

    int get_sample_count();
    void push_event_internal(tap_event_t event);
    
    TapUser() = default;
};