
#include "core/object/class_db.h"
#include "editor/editor_node.h"

#include "tap_sim.h"
#include "tap_sim_generator.h"

void TapSimGenerator::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_sim_path"), &TapSimGenerator::get_sim_path);
  ClassDB::bind_method(D_METHOD("set_sim_path", "new_sim_path"), &TapSimGenerator::set_sim_path);

  ClassDB::bind_method(D_METHOD("get_input_wavs"), &TapSimGenerator::get_input_wavs);
  ClassDB::bind_method(D_METHOD("set_input_wavs", "new_input_wavs"), &TapSimGenerator::set_input_wavs);

  ClassDB::bind_method(D_METHOD("get_output_pids"), &TapSimGenerator::get_output_pids);
  ClassDB::bind_method(D_METHOD("set_output_pids", "new_output_pids"), &TapSimGenerator::set_output_pids);

  ClassDB::bind_method(D_METHOD("get_player_output_pids"), &TapSimGenerator::get_player_output_pids);
  ClassDB::bind_method(D_METHOD("set_player_output_pids", "new_player_output_pids"), &TapSimGenerator::set_player_output_pids);

  ClassDB::bind_method(D_METHOD("get_tick_rate"), &TapSimGenerator::get_tick_rate);
  ClassDB::bind_method(D_METHOD("set_tick_rate", "new_tick_rate"), &TapSimGenerator::set_tick_rate);

  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "sim_path", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "TapSim"), "set_sim_path", "get_sim_path");
  ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "input_wavs", PROPERTY_HINT_DICTIONARY_TYPE, String("int:") + SourceT::get_class_static()), "set_input_wavs", "get_input_wavs");
  ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT64_ARRAY, "output_pids"), "set_output_pids", "get_output_pids");
  ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT64_ARRAY, "player_output_pids"), "set_player_output_pids", "get_player_output_pids");
  ADD_PROPERTY(PropertyInfo(Variant::INT, "tick_rate"), "set_tick_rate", "get_tick_rate");
}

NodePath TapSimGenerator::get_sim_path() const {
  return sim_path;
}

void TapSimGenerator::set_sim_path(NodePath new_sim_path) {
  sim_path = new_sim_path;
}

Dictionary TapSimGenerator::get_input_wavs() const {
  Dictionary result;

  for (auto pair : input_wavs) {
    result.set(pair.key, pair.value);
  }

  return result;
}

void TapSimGenerator::set_input_wavs(const Dictionary &new_input_wavs) {
  input_wavs.clear();
  
  for (auto key : new_input_wavs.keys()) {
    bool bad = false;

    if (key.get_type() != Variant::INT) {
      WARN_PRINT(String("Key type ") + Variant::get_type_name(key) + " should be int.");
      bad = true;
    }

    Variant value = new_input_wavs[key];
    if (value.get_type() != Variant::OBJECT || !Object::cast_to<SourceT>(value)) {
      WARN_PRINT(String("Value type " + Variant::get_type_name(value) + " could not be cast to " + SourceT::get_class_static()));
      bad = true;
    }

    if (bad) {
      continue;
    }

    input_wavs[key] = Object::cast_to<SourceT>(value);
  }
}

PackedInt64Array TapSimGenerator::get_output_pids() const {
  return output_pids;
}

void TapSimGenerator::set_output_pids(const PackedInt64Array &new_output_pids) {
  output_pids = new_output_pids;
}

PackedInt64Array TapSimGenerator::get_player_output_pids() const {
  return player_output_pids;
}

void TapSimGenerator::set_player_output_pids(const PackedInt64Array &new_player_output_pids) {
  player_output_pids = new_player_output_pids;
}

int TapSimGenerator::get_tick_rate() const {
  return tick_rate;
}

void TapSimGenerator::set_tick_rate(int new_tick_rate) {
  tick_rate = new_tick_rate;
}

Ref<AudioStreamPlayback> TapSimGenerator::instantiate_playback() {
  Ref<TapSimGeneratorPlayback> result;
  result.instantiate();

  Node *es = EditorNode::get_singleton()->get_edited_scene();
  Node *sim = es->get_node(sim_path);

  if (Object::cast_to<TapSim>(sim)) {
    result->sim = Object::cast_to<TapSim>(sim);
  }
  
  result->input_wavs = input_wavs;
  result->output_pids = output_pids;
  result->player_output_pids = player_output_pids;
  result->tick_rate = tick_rate;

  return result;
}