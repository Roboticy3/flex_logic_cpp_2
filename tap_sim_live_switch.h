
#include "core/io/resource.h"
#include "core/variant/variant.h"

#include "tap_sim.h"

/**
 * @brief Separate the logic of checking if a TapSim can be simulated (or `live`)
 * from the AudioEffectTapOut class, since it can be reused by AudioEffectTapDebugger
 */
class TapSimLiveSwitch {

  private:
    bool live = false;
    Ref<TapSim> simulator;
    PackedInt64Array live_pids;
  
  public:

    bool get_live() const;
    /**
     * @brief Only allow live to be set to true if `simulator` is valid an all
     * `live_pids` are valid pins inside that simulator.
     */
    void set_live(bool new_live);

    Ref<TapSim> get_simulator() const;
    /**
     * @brief Update the simulator and reset `live` and `live_pids`.
     */
    void set_simulator(Ref<TapSim> new_simulator);

    PackedInt64Array get_live_pids() const;
    /**
     * @brief Set `live_pids` and set `live` to false if it was previously true
     * and these new pids are invalid on the current simulator.
     */
    void set_live_pids(PackedInt64Array new_live_pids);
};