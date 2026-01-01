#pragma once

#include "core/object/class_db.h"
#include "core/io/resource.h"

#include "tap_circuit_types.h"
#include "tap_user.h"

/*
Define a resource wrapper for tap_component_type_t, allowing the user to 
dynamically specify type names, sensitive pin indices, and solver functions via
a StringName. The solver stringname looks into a static registry of functions
belonging to this resource class. 
*/
class TapComponentType : public Resource {
  GDCLASS(TapComponentType, Resource)

  tap_component_type_t component_type;
  StringName solver_function_name;

  static HashMap<StringName, tap_component_type_t::solver_t> solver_registry;

  protected:
    static void _bind_methods();
  
  public:
    void set_type_name(StringName new_name);
    StringName get_type_name();

    void set_sensitive_pins(const Vector<int> &new_sensitive_pins);
    Vector<int> get_sensitive_pins();

    void set_solver_function(StringName solver_name);
    StringName get_solver_function_name();

    const tap_component_type_t &get_component_type_internal() const;
    
    static void initialize_solver_registry_internal();
    static void uninitialize_solver_registry_internal();

    TapComponentType() = default;
};