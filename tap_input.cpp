#include "tap_input.h"
#include "core/object/class_db.h"

void TapInput::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_stream"), &TapInput::get_stream);
  ClassDB::bind_method(D_METHOD("set_stream", "stream"), &TapInput::set_stream);
  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "stream", PROPERTY_HINT_RESOURCE_TYPE, "AudioStream"), "set_stream", "get_stream");

  ClassDB::bind_method(D_METHOD("get_pid"), &TapInput::get_pid);
  ClassDB::bind_method(D_METHOD("set_pid", "pid"), &TapInput::set_pid);
  ADD_PROPERTY(PropertyInfo(Variant::INT, "pid", PROPERTY_HINT_NONE, ""), "set_pid", "get_pid");

  ClassDB::bind_method(D_METHOD("get_inner_loop_count"), &TapInput::get_inner_loop_count);
  ClassDB::bind_method(D_METHOD("set_inner_loop_count", "proportion"), &TapInput::set_inner_loop_count);
  ADD_PROPERTY(PropertyInfo(Variant::INT, "inner_loop_count", PROPERTY_HINT_NONE, ""), "set_inner_loop_count", "get_inner_loop_count");
}

TapInput::TapInput() {
}

TapInput::TapInput(Ref<AudioStream> p_stream, tap_label_t p_pid, int p_inner_loop_count) {
  stream = p_stream;
  pid = p_pid;
  inner_loop_count = p_inner_loop_count;
}

Ref<AudioStream> TapInput::get_stream() const {
  return stream;
}

void TapInput::set_stream(Ref<AudioStream> p_stream) {
  stream = p_stream;
}

tap_label_t TapInput::get_pid() const {
  return pid;
}

void TapInput::set_pid(tap_label_t p_pid) {
  pid = p_pid;
}

int TapInput::get_inner_loop_count() const {
  return inner_loop_count;
}

void TapInput::set_inner_loop_count(int p_inner_loop_count) {
  inner_loop_count = p_inner_loop_count;
}
