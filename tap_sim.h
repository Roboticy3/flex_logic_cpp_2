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
class TapSim : public Node {
  GDCLASS(TapSim, Node);

  Ref<TapNetwork> network;
  Ref<TapPatchBay> patch_bay; //currently, network composes patch bay, but don't want to rely on that

  protected:
    static void _bind_methods();

  public:
    Ref<TapNetwork> get_network() const;
    void set_network(Ref<TapNetwork> new_network);

    Ref<TapPatchBay> get_patch_bay() const;
    void set_patch_bay(Ref<TapPatchBay> new_patch_bay);

    /*
    Process the top event in the event queue.
    */
    void process_once_internal(tap_queue_t &queue);
    /*
    Expose to the editor as automatically using patch_bay->get_queue_internal()
    Useful for manual stepping in the editor.
    */
    void process_once();
};