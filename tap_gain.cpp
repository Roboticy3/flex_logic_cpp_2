

#include "core/math/math_funcs.h"
#include "core/typedefs.h"

#include "tap_gain.h"

int tap_gain_solver_calls = 0;

void TapGain::solver(tap_label_t cid, const tap_component_t &component, const Vector<const tap_event_t *> &state, tap_queue_t &queue, tap_time_t current_time) {
  
  if (component.memory_size < 2) {
    ERR_PRINT("TapGain::solver Memory size must be at least 2 (was " + itos(component.memory_size) + ")");
    return;
  }
  
  tap_frame_t s = component.memory[0];
  tap_frame_t push_out = component.memory[1];
  tap_frame_t push_in = state[0]->state;
  size_t window_size = component.memory_size - 1;
  
  tap_frame_t os = push_out * push_out;
  tap_frame_t is = push_in * push_in;
  tap_frame_t next_s = s + (is - os);
  tap_frame_t next_ms = next_s / (float)window_size;
  tap_frame_t next_rms = {Math::sqrt(next_ms.l), Math::sqrt(next_ms.r)};
  tap_frame_t next_power = {
    Math::pow(next_rms.l, POWER),
    Math::pow(next_rms.r, POWER)
  };

  component.memory[0] = next_s;

  for (size_t i = 1; i < component.memory_size - 1; i++) {
    component.memory[i] = component.memory[i + 1];
  }

  component.memory[component.memory_size - 1] = push_in;

  /*
  tap_gain_solver_calls++;
  if (tap_gain_solver_calls % 1000 == 0) {
    std::cout << "rms: " << component.memory[0].l << ", " << component.memory[0].r << std::endl;
  }
  */

  tap_time_t next_time = current_time + component.memory_size;
  queue.insert(tap_event_t{next_time, next_power, state[1]->pid, cid}, next_time);
}
