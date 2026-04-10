#pragma once

#include "core/object/ref_counted.h"
#include "core/templates/ring_buffer.h"
#include "core/math/audio_frame.h"
#include "core/string/print_string.h"
#include "core/templates/vector.h"
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

  void set_total_error(AudioFrame p_total_error) { 
    total_error = p_total_error; 
    success = total_error.r + total_error.l <= tolerance;
  }
  AudioFrame get_total_error() const { return total_error; }

protected:
  static void _bind_methods();
};

class ReferenceFrameStack {
  RingBuffer<Ref<ReferenceFrame>> frames;
  std::recursive_mutex mutex;

public:
  ReferenceFrameStack() {
    frames.resize(10);
  }

  void push(const Ref<ReferenceFrame>& frame) { 
    std::lock_guard<std::recursive_mutex> lock(mutex); 
    frames.write(frame); 
  }
  void pop() { 
    std::lock_guard<std::recursive_mutex> lock(mutex); 
    frames.read(); 
  }
  void resize(int p_power) { 
    std::lock_guard<std::recursive_mutex> lock(mutex); 
    frames.resize(p_power); 
  }
  Ref<ReferenceFrame> top() { 
    std::lock_guard<std::recursive_mutex> lock(mutex); 
    
    // Debug: Print ring buffer contents
    print_line("=== ReferenceFrameStack Debug ===");
    print_line("Ring buffer size: " + itos(frames.size()));
    print_line("Data left: " + itos(frames.data_left()));
    
    // Create a temporary buffer to copy ring buffer contents
    Vector<Ref<ReferenceFrame>> temp_buffer;
    temp_buffer.resize(frames.data_left());
    int copied = frames.copy(temp_buffer.ptrw(), 0, frames.data_left());
    
    for (int i = 0; i < copied; i++) {
      Ref<ReferenceFrame> frame = temp_buffer[i];
      if (frame.is_valid()) {
        AudioFrame error = frame->get_total_error();
        print_line("Frame " + itos(i) + ": success=" + (frame->get_success() ? "true" : "false") + 
                  ", error=(" + rtos(error.l) + ", " + rtos(error.r) + ")");
      } else {
        print_line("Frame " + itos(i) + ": null");
      }
    }
    print_line("================================");
    
    return frames.read(); 
  }
};