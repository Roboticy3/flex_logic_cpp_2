#include "tap_input.h"
#include "core/object/class_db.h"

void TapInput::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_stream"), &TapInput::get_stream);
  ClassDB::bind_method(D_METHOD("set_stream", "stream"), &TapInput::set_stream);
  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "stream", PROPERTY_HINT_RESOURCE_TYPE, "AudioStream"), "set_stream", "get_stream");

  ClassDB::bind_method(D_METHOD("get_pid"), &TapInput::get_pid);
  ClassDB::bind_method(D_METHOD("set_pid", "pid"), &TapInput::set_pid);
  ADD_PROPERTY(PropertyInfo(Variant::INT, "pid", PROPERTY_HINT_NONE, ""), "set_pid", "get_pid");

  ClassDB::bind_method(D_METHOD("get_verification_proportion"), &TapInput::get_verification_proportion);
  ClassDB::bind_method(D_METHOD("set_verification_proportion", "proportion"), &TapInput::set_verification_proportion);
  ADD_PROPERTY(PropertyInfo(Variant::INT, "verification_proportion", PROPERTY_HINT_NONE, ""), "set_verification_proportion", "get_verification_proportion");
}

TapInput::TapInput() {
}

TapInput::TapInput(Ref<AudioStream> p_stream, tap_label_t p_pid, int p_verification_proportion) {
  stream = p_stream;
  pid = p_pid;
  verification_proportion = p_verification_proportion;
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

int TapInput::get_verification_proportion() const {
  return verification_proportion;
}

void TapInput::set_verification_proportion(int p_verification_proportion) {
  verification_proportion = p_verification_proportion;
}
