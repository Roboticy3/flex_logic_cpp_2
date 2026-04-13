#include "tap_gain.h"
#include <iostream>

void TapGain::solver(tap_label_t cid, const tap_component_t &component, const Vector<const tap_event_t *> &state, tap_queue_t &queue, tap_time_t current_time) {
  std::cout << "Memory size: " << component.memory_size << std::endl;
  std::cout << "Memory sample: (" << component.memory[0].l << ", " << component.memory[0].r << "), (" << component.memory[1].l << ", " << component.memory[1].r << "), (" << component.memory[2].l << ", " << component.memory[2].r << ")" << std::endl;
}
