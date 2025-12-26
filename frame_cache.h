#pragma once

#include "core/object/ref_counted.h"
#include "core/io/resource.h"
#include "core/templates/vector.h"
#include "core/math/audio_frame.h"

/*
DEPRECATED

Store a frame buffer from a cache.
*/
class FrameCache : public Resource {
  GDCLASS(FrameCache, Resource);

  Vector<AudioFrame> frame_buffer;
  uint32_t frame_id;

  static constexpr uint32_t MIN_CACHE_SIZE = 1024;

  protected:
    static void _bind_methods();

  public:
    void ensure_size(uint32_t p_size);
    uint32_t get_size() const;

    void increment_frame_id();
    uint32_t get_frame_id() const;

    AudioFrame *get_frame_buffer_internal();

    FrameCache();
};