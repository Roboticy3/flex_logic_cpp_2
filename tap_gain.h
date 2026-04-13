#pragma once

#include "tap_circuit_types.h"

class TapGain {
  
public:
  static void solver(tap_label_t cid, const tap_component_t &component, const Vector<const tap_event_t *> &state, tap_queue_t &queue, tap_time_t current_time);
};