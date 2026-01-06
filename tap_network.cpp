
#include "core/object/class_db.h"
#include "core/variant/variant.h"

#include "tap_network.h"

void TapNetwork::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_invalid_label"), &TapNetwork::get_invalid_label);
  ClassDB::bind_method(D_METHOD("set_patch_bay", "patch_bay"), &TapNetwork::set_patch_bay);
  ClassDB::bind_method(D_METHOD("get_patch_bay"), &TapNetwork::get_patch_bay);

  ClassDB::bind_method(D_METHOD("set_component_types", "component_types"), &TapNetwork::set_component_types);
  ClassDB::bind_method(D_METHOD("get_component_types"), &TapNetwork::get_component_types);

  ClassDB::bind_method(D_METHOD("set_wire_type", "wire_type"), &TapNetwork::set_wire_type);
  ClassDB::bind_method(D_METHOD("get_wire_type"), &TapNetwork::get_wire_type);

  ClassDB::bind_method(D_METHOD("add_component", "pin_labels", "component_type_index"), &TapNetwork::add_component);
  ClassDB::bind_method(D_METHOD("get_component_type", "component_label"), &TapNetwork::get_component_type);
  ClassDB::bind_method(D_METHOD("move_component", "component_label", "new_pin_labels"), &TapNetwork::move_component);
  ClassDB::bind_method(D_METHOD("remove_component", "component_label"), &TapNetwork::remove_component);

  ClassDB::bind_method(D_METHOD("get_all_component_connections"), &TapNetwork::get_all_component_connections);
  ClassDB::bind_method(D_METHOD("get_all_component_types"), &TapNetwork::get_all_component_types);

  ADD_PROPERTY(PropertyInfo(Variant::INT, "invalid_label"), "", "get_invalid_label");

  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "patch_bay", PROPERTY_HINT_RESOURCE_TYPE, "TapPatchBay"), "set_patch_bay", "get_patch_bay");
  ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "component_types", PROPERTY_HINT_ARRAY_TYPE, "TapComponentType"), "set_component_types", "get_component_types");
  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "wire_type", PROPERTY_HINT_RESOURCE_TYPE, "TapComponentType"), "set_wire_type", "get_wire_type");
}

tap_label_t TapNetwork::get_invalid_label() const {
  return components.INVALID_LABEL;
}

void TapNetwork::set_patch_bay(Ref<TapPatchBay> p_patch_bay) {
  patch_bay = p_patch_bay;
}

Ref<TapPatchBay> TapNetwork::get_patch_bay() const {
  return patch_bay;
}

void TapNetwork::set_component_types(Array p_component_types) {
  component_types.clear();
  for (int i = 0; i < p_component_types.size(); i++) {
    TapComponentType *ct = Object::cast_to<TapComponentType>(p_component_types[i]);
    if (ct) {   
      std::optional<Ref<TapComponentType>> o_type = Ref<TapComponentType>(ct);
      component_types.push_back(o_type);
    } else {
      component_types.push_back(std::nullopt);
    }
  }
}

Array TapNetwork::get_component_types() const {
  Array arr;
  for (int i = 0; i < component_types.size(); i++) {
    if (component_types[i].has_value()) 
      arr.push_back(component_types[i].value());
    else {
      arr.push_back(Variant());
    }
  }
  return arr;
}

Ref<TapComponentType> TapNetwork::get_wire_type() const {
  return wire_component_type;
}

void TapNetwork::set_wire_type(Ref<TapComponentType> wire_type) {
  wire_component_type = wire_type;
}

Vector<tap_label_t> TapNetwork::validate_pin_labels(PackedInt64Array pin_labels) const {
  Vector<tap_label_t> valid_labels;
  HashSet<tap_label_t> label_set;
  
  //if the patch bay is not set, return empty and print an error
  if (patch_bay.is_null()) {
    ERR_PRINT("TapNetwork::validate_pin_labels: patch bay is not set. Components cannot be connected.");
    return valid_labels;
  }
  
  for (int i = 0; i < pin_labels.size(); i++) {
    tap_label_t label = static_cast<tap_label_t>(pin_labels[i]);

    //validate the pin is unique
    if (!label_set.has(label)) {
      label_set.insert(label);
    } else {
      continue;
    }

    //validate the pin exists in the patch bay
    if (!patch_bay->has_pin(label)) {
      continue;
    }

    valid_labels.push_back(label);
  }

  return valid_labels;
}

tap_component_t TapNetwork::validate_pin_labels_and_type(PackedInt64Array pin_labels, tap_label_t component_type_index) const {
  Vector<tap_label_t> valid_labels = validate_pin_labels(pin_labels);

  tap_component_t component;

  auto component_type = component_types.label_get(component_type_index).value_or(Ref<TapComponentType>());
  auto ct_internal = component_type.is_valid() ? component_type->get_component_type_internal() : tap_component_type_t();

  //determine component type
  if (ct_internal.pin_count == 0) {
    //wire type
    if (valid_labels.size() == 0) {
      ERR_PRINT("TapNetwork::validate_pin_labels_and_type: wire components must have at least one pin.");
      return component;
    }
    component.component_type = wire_component_type->get_component_type_internal();
  } else {

    if (valid_labels.size() != component_type->get_component_type_internal().pin_count) {
      ERR_PRINT("TapNetwork::validate_pin_labels_and_type: pin count does not match component type.");
      return component;
    }

    component.component_type = component_type->get_component_type_internal();
  }

  component.pins = valid_labels;
  return component;
}

tap_label_t TapNetwork::add_component(PackedInt64Array pin_labels, tap_label_t component_type_index) {

  tap_component_t component = validate_pin_labels_and_type(pin_labels, component_type_index);
  if (component.pins.size() == 0) {
    return components.INVALID_LABEL;
  }

  tap_label_t label = components.label_add(component);

  //!TODO! attach pins to the component

  return label;
}

Ref<TapComponentType> TapNetwork::get_component_type(tap_label_t component_label) {
  std::optional<tap_component_t> o_component = components.label_get(component_label);
  if (!o_component.has_value()) {
    return Ref<TapComponentType>();
  }
  tap_component_t component = o_component.value();

  for (int i = 0; i < component_types.size(); i++) {
    if (component_types[i].has_value() == false) {
      continue;
    }

    auto ct = component_types[i].value();
    if (ct->get_component_type_internal().name == component.component_type.name) {
      return ct;
    }
  }

  if (component.component_type.name == wire_component_type->get_component_type_internal().name) {
    return wire_component_type;
  }

  return Ref<TapComponentType>();
}

bool TapNetwork::move_component(tap_label_t component_label, PackedInt64Array new_pin_labels) {
  std::optional<tap_component_t> o_component = components.label_get(component_label);
  if (!o_component.has_value()) {
    return false;
  }
  tap_component_t component = o_component.value();

  tap_component_t destination_component = validate_pin_labels_and_type(new_pin_labels, WIRE_TYPE);
  if (destination_component.pins.size() != component.pins.size()) {
    return false;
  }

  //!TODO! detach old pins from component

  //attach component to new pins
  component.pins = destination_component.pins;

  //!TODO! attach new pins to component
  return true;
}

bool TapNetwork::remove_component(tap_label_t component_label) {
  bool result =components.label_remove(component_label);
  
  if (result) {
    //!TODO! detach pins from component
  }

  return result;
}

Array TapNetwork::get_all_component_connections() const {
  Array arr;

  for (auto o_component : components) {
    PackedInt64Array pin_labels;
    if (o_component.has_value()) {
      tap_component_t component = o_component.value();
      for (auto label : component.pins) {
        pin_labels.push_back(static_cast<int64_t>(label));
      }
    }
    arr.push_back(pin_labels);
  }
  return arr;
}

Array TapNetwork::get_all_component_types() const {
  Array arr;
  for (auto o_component : components) {
    tap_label_t found_type = -1;
    if (o_component.has_value()) {
      tap_component_t component = o_component.value();

      for (int i = 0; i < component_types.size(); i++) {
        auto o_component_type = component_types.label_get(i);
        if (o_component_type.has_value()) {
          Ref<TapComponentType> component_type = o_component_type.value();
          if (component_type->get_component_type_internal().name == component.component_type.name) {
            
            found_type = i;
            break;
          }
        }
      }
    }

    arr.push_back(found_type);
  }
  return arr;
}
  