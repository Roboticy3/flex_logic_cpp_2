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

  protected:
    static void _bind_methods();

  public:
    Ref<TapNetwork> get_network() const;
    void set_network(Ref<TapNetwork> new_network);

    Ref<TapPatchBay> get_patch_bay() const;
    void set_patch_bay(Ref<TapPatchBay> new_patch_bay);

    int get_tick_rate() const;
    void set_tick_rate(int new_tick_rate);

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
    */
    void process_to(tap_time_t end_time);

    /*
    Automatically set up a simulator with a minimally configured network + patch

    Comes with:
     - Populated network and patch bay
     - Wire type for added components to default to
    */
    TapSim();
};