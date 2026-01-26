#pragma once

#include "core/object/class_db.h"
#include "core/object/ref_counted.h"
#include "scene/main/node.h"

#include "tap_patch_bay.h"
#include "tap_network.h"

/*
Aggregate a TapNetwork and TapPatchBay to a full circuit. Resolve state manually
or by giving to a TapSimAudioStreamGenerator
*/
class TapSim : public Resource {
  GDCLASS(TapSim, Resource);

  Ref<TapNetwork> network;
  Ref<TapPatchBay> patch_bay; //currently, network composes patch bay, but don't want to rely on that

  int tick_rate = 1024;
  tap_time_t latest_event_time = 0;

  protected:
    static void _bind_methods();

  public:
    Ref<TapNetwork> get_network() const;
    void set_network(Ref<TapNetwork> new_network);

    Ref<TapPatchBay> get_patch_bay() const;
    void set_patch_bay(Ref<TapPatchBay> new_patch_bay);

    int get_tick_rate() const;
    void set_tick_rate(int new_tick_rate);

    tap_time_t get_latest_event_time() const;

    /*
    Process an event with a priority queue as the source. Pops the top event
    off of the queue.
    */
    void process_once_internal(tap_queue_t &queue);

    /*
    Process an event with the TapSim's configured `patch_bay` as the source for
    a queue, and thus the next event. Since this does not take any internal
    types as an argument, it can be exposed to the editor.
    */
    void process_once();

    /*
    Proper simulation function. As opposed to the traditional timestep, pass a 
    total time target.

    Returns the number of events processed.
    */
    int process_to(tap_time_t end_time);

    /*
    Push an event and update the internal `latest_event_time` value, which users
    can read to check if there are enough events to simulate to a certain time.
    */
    void push_event(tap_time_t time, AudioFrame state, tap_label_t pid);

    /*
    Automatically set up a simulator with a minimally configured network + patch

    Comes with:
     - Populated network and patch bay
     - Wire type for added components to default to
    */
    TapSim();
};