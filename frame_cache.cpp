#include "core/object/class_db.h"

#include "frame_cache.h"

void FrameCache::_bind_methods() {
  ClassDB::bind_method(D_METHOD("ensure_size", "size"), &FrameCache::ensure_size);
  ClassDB::bind_method(D_METHOD("increment_frame_id"), &FrameCache::increment_frame_id);

  ClassDB::bind_method(D_METHOD("get_size"), &FrameCache::get_size);
  ClassDB::bind_method(D_METHOD("get_frame_id"), &FrameCache::get_frame_id);
}

void FrameCache::ensure_size(uint32_t p_size) {
  if (p_size > frame_buffer.size()) {
    frame_buffer.resize(p_size);
  }
}

uint32_t FrameCache::get_size() const {
  return frame_buffer.size();
}

void FrameCache::increment_frame_id() {
  frame_id++;
}

uint32_t FrameCache::get_frame_id() const {
  return frame_id;
}

AudioFrame *FrameCache::get_frame_buffer_internal() {
  return frame_buffer.ptrw();
}

FrameCache::FrameCache() : frame_id(0) {
  frame_buffer.clear();
  frame_buffer.resize(MIN_CACHE_SIZE);
}