#pragma once

#undef likely
#undef unlikely
#undef DEBUG_ENABLED
#include "thirdparty/harfbuzz/src/hb-priority-queue.hh"

#include "core/string/string_name.h"
#include "core/templates/vector.h"
#include "core/templates/span.h"

template<typename T, typename IDIn>
struct circuit_event_t {
  T time;
  IDIn id;

  inline constexpr bool operator<=(const circuit_event_t &other) const {
    return time <= other.time;
  }
  inline constexpr bool operator<(const circuit_event_t &other) const {
    return time < other.time;
  }
};

template<typename T, typename IDIn>
using circuit_queue_t = hb_priority_queue_t<circuit_event_t<T, IDIn>>;

template<typename S, typename IDOut>
struct circuit_pin_t {
  S state;
  Span<IDOut> drivers;
};

template<typename S, typename T, typename IDOut, typename IDIn>
struct circuit_component_t {
  /*
  "create events from input connections and output connections by sampling full 
  list of driver pins, then pushing events to a queue"

  This should be enough to data to propogate events. The circuit solver reads 
  circuit events on pins.
  */
  void(*solver)(const Span<IDIn> &inputs, const Span<IDOut> &outputs, const Vector<circuit_pin_t<S, IDIn>> &pins, circuit_queue_t<S, T> &output_queue);
  StringName name;
  Vector<IDIn> sensitive;
};