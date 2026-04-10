#pragma once

#include "core/object/ref_counted.h"
#include "core/math/audio_frame.h"
#include "core/templates/local_vector.h"
#include <mutex>

class ReferenceFrame : public RefCounted {
  GDCLASS(ReferenceFrame, RefCounted);
  
  bool success;
  float tolerance;
  AudioFrame total_error;

public:
  ReferenceFrame(AudioFrame total_error = AudioFrame(0.0f, 0.0f), float tolerance = 0.01f) : tolerance(tolerance), total_error(total_error) {
    success = total_error.r + total_error.l <= tolerance;
  }

  bool get_success() const { return success; }

  void set_tolerance(float p_tolerance) { 
    tolerance = p_tolerance; 
    success = total_error.r + total_error.l <= tolerance;
  }
  float get_tolerance() const { return tolerance; }

  void set_total_error(Vector2 p_total_error) { 
    total_error = AudioFrame(p_total_error.x, p_total_error.y); 
    success = total_error.r + total_error.l <= tolerance;
  }
  Vector2 get_total_error() const { return Vector2(total_error.l, total_error.r); }

protected:
  static void _bind_methods();
};

class ReferenceFrameContainer {
  LocalVector<Ref<ReferenceFrame>> frames;
  std::recursive_mutex mutex;

public:
  void push(const Ref<ReferenceFrame>& frame) { 
    std::lock_guard<std::recursive_mutex> lock(mutex); 
    frames.push_back(frame); 
  }
  void pop() { 
    std::lock_guard<std::recursive_mutex> lock(mutex); 
    frames.remove_at(frames.size() - 1); 
  }
  Ref<ReferenceFrame> top() { 
    std::lock_guard<std::recursive_mutex> lock(mutex); 
    if (frames.size() == 0) {
      return Ref<ReferenceFrame>();
    }
    return frames[frames.size() - 1]; 
  }

  void clear() { 
    std::lock_guard<std::recursive_mutex> lock(mutex); 
    frames.clear(); 
  }
};
