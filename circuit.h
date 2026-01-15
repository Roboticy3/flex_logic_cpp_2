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

The source component id must also be stored to avoid infinite loops. i.e. if a
component sends an event to a pin, the solver needs to see that even and know
not to "bounce" the event back into the same component.
*/
template<typename S, typename T, typename PinID, typename ComponentID>
struct circuit_event_t {
  T time;
  S state;
  PinID pid;
  ComponentID source_cid;


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
template<typename S, typename T, typename PinID, typename ComponentID>
using circuit_queue_t = hb_priority_queue_t<circuit_event_t<S, T, PinID, ComponentID>>;

/*
A link between components in the circuit, along with the last event that passed
through this point.
*/
template<typename S, typename T, typename ComponentID>
struct circuit_pin_t {
  Vector<ComponentID> components;
};

/*
Define a component type in a circuit. The solver should be capable of processing 
any component at any time based on its current state and push events to a
destination queue.

`name` : identifying name for this type
`sensitive` : indices from 0 to `pin_count-1` where events should induce a call
 to solver
`pin_count` : number of pins this component has. If variable, set to 0.
`solver` : function pointer to the solver function for this type
*/
template<typename T, typename ComponentID, typename EventT, typename QueueT>
struct circuit_component_type_t {
  using solver_t = void(*)(const Vector<EventT> &state, QueueT &queue, T current_time, ComponentID cid);

  StringName name;
  Vector<int> sensitive;
  int pin_count;
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
template<typename S, typename T, typename PinID, typename ComponentID, typename EventT, typename QueueT>
struct circuit_component_t {
  /*
  What is this component's name, pinout, and solver.
  */
  circuit_component_type_t<T, ComponentID, EventT, QueueT> component_type;
  /*
  The pin ids connected to this component. Since pins store their last event,
  this also defines the state.
  */
  Vector<PinID> pins;
  /*
  The component's internal memory. Currently unused.
  */
  Vector<S> memory; 

  template<typename F, typename... X>
  inline void for_each_sensitive(F&& func, X&&... varargs) const {
    if (component_type.sensitive.is_empty()) {
      for (int i = 0; i < pins.size(); i++) {
        func(pins[i], std::forward<X>(varargs)...);
      }
    } else {
      for (int i : component_type.sensitive) {
        func(pins[i], std::forward<X>(varargs)...);
      }
    }
  }
};
