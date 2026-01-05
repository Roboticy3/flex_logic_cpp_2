#pragma once

#undef likely
#undef unlikely
#undef DEBUG_ENABLED
#include "thirdparty/harfbuzz/src/hb-priority-queue.hh"

#include "core/string/string_name.h"
#include "core/templates/vector.h"
#include "core/templates/span.h"

/*
A singular event ona circuit, happening on a pin at a certain time with a 
certain state. By collecting events, one can derive the entire current state of
a component.

Pins can also store events, making it possible to locate a component's drivers
without storing an additional vector.
*/
template<typename S, typename T, typename PinID>
struct circuit_event_t {
  T time;
  S state;
  PinID pid;

  inline constexpr bool operator<=(const circuit_event_t &other) const {
    return time <= other.time;
  }
  inline constexpr bool operator<(const circuit_event_t &other) const {
    return time < other.time;
  }
};

/*
A priority queue of circuit events, implemented by harfbuzz :)
*/
template<typename S, typename T, typename PinID>
using circuit_queue_t = hb_priority_queue_t<circuit_event_t<S, T, PinID>>;

/*
A link between components in the circuit, along with the last event that passed
through this point.
*/
template<typename S, typename T, typename ComponentID, typename PinID>
struct circuit_pin_t {
  Vector<ComponentID> components;
  PinID id;
};

/*
Define a component type in a circuit. The solver should be capable of processing 
any component at any time based on its current state and push events to a
destination queue.
*/
template<typename T, typename EventT, typename QueueT>
struct circuit_component_type_t {
  using solver_t = void(*)(const Vector<EventT> &state, QueueT &queue, T current_time);

  StringName name;
  Vector<int> sensitive;
  //state vector corresponds to sensitive pins
  solver_t solver;
};

/*
Define a component instance in a circuit. Has a type, which defines how to 
handle the component's state. Components also connect to pins via `PinID`, and
may contain an internal memory of type `S`- the same type as the state in the
events that the component processes.

`A` is the component type identifier, of which `circuit_component_type_t` is an
example.
*/
template<typename S, typename PinID, typename A>
struct circuit_component_t {
  /*
  What is this component's name, pinout, and solver.
  */
  A component_type;
  /*
  The pin ids connected to this component. Since pins store their last event,
  this also defines the state.
  */
  Vector<PinID> pins;
  /*
  The component's internal memory.
  */
  Vector<S> memory; 
};
