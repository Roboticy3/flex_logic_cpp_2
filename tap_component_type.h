#pragma once

#include "core/object/class_db.h"
#include "core/io/resource.h"

#include "tap_circuit_types.h"
#include "tap_patch_bay.h"

void wire_solver(const Vector<const tap_event_t *> &state, tap_queue_t &queue, tap_time_t current_time, tap_label_t cid);

void adder_solver(const Vector<const tap_event_t *> &pins, tap_queue_t &queue, tap_time_t current_time, tap_label_t cid);

/*
Define a resource wrapper for tap_component_type_t, allowing the user to 
dynamically specify type names, sensitive pin indices, and solver functions via
a StringName. The solver stringname looks into a static registry of functions
belonging to this resource class. 
*/
class TapComponentType : public Resource {
  GDCLASS(TapComponentType, Resource)

  tap_component_type_t component_type = {
    "Wire",
    Vector<int>(), //empty mask => all pins sensitive
    0, //pin count of 0 means variable
    &wire_solver, //default to wire solver
  };
  StringName solver_function_name = "wire";

  protected:
    static void _bind_methods();
  
  public:
    void set_type_name(StringName new_name);
    StringName get_type_name();

    void set_sensitive_pins(const Vector<int> &new_sensitive_pins);
    Vector<int> get_sensitive_pins();

    void set_pin_count(int new_pin_count);
    int get_pin_count() const;

    void set_solver_function(StringName solver_name);
    StringName get_solver_function_name();

    void set_component_type_internal(tap_component_type_t new_component_type);
    tap_component_type_t get_component_type_internal() const;
  
    
    static void initialize_solver_registry_internal();
    static void uninitialize_solver_registry_internal();

    static HashMap<StringName, tap_component_type_t::solver_t> solver_registry;

    TapComponentType() = default;
};