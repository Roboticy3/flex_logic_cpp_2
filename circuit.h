#pragma once

#undef likely
#undef unlikely
#undef DEBUG_ENABLED
#include "thirdparty/harfbuzz/src/hb-priority-queue.hh"

#include "core/string/string_name.h"
#include "core/templates/vector.h"
#include "core/templates/span.h"

template<typename S, typename T>
struct circuit_event_t {
  unsigned id;
  S state;
  T time;
};

template<typename S, typename T>
using circuit_queue_t = hb_priority_queue_t<circuit_event_t<S, T>>;

template<typename S, typename T, typename ID>
struct circuit_component_t {
  void(*solver)(Vector<S> inputs, int change, Span<ID> pins, circuit_queue_t<S, T> &queue);
  StringName name;
};