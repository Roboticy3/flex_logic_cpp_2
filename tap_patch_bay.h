#pragma once

#include <optional>

#include "core/io/resource.h"
#include "core/variant/variant.h"
#include "core/variant/typed_dictionary.h"
#include "core/templates/vector.h"

#include "labeling.h"
#include "tap_circuit_types.h"


/**
 * @brief Store templates for circuit primitives that can be attached to an audio tap.
 * 
 * Not responsible for event propagation, only maintaining pins and event queue.
 * 
 * Also used to expose basic event operations to the editor. Unregistered methods
 * will be named *_internal.
 * 
 * Queue is a protected member for easy access during simulation.
 */
class TapPatchBay : public Resource {
  GDCLASS(TapPatchBay, Resource)

  /// @brief Number of simulation ticks per audio sample
  int tick_rate = 1024;

  /// @brief Event queue
  tap_queue_t queue;

  /// @brief Pin mapping
  Labeling<tap_pin_t> pins;

  /// @brief State mapping (keep separate from optional pins for easier access)
  Vector<tap_event_t> pin_states;

  protected:
    static void _bind_methods();

  public:

    /// @brief Constant indicating component is missing (narrowing conversion my ass)
    static constexpr tap_label_t COMPONENT_MISSING = -1;

    static constexpr Vector2i STATE_MISSING = Vector2i(2, 2);
    inline Vector2i get_state_missing() const {
      return STATE_MISSING;
    }

    void push_event(tap_time_t time, Vector2 levels, tap_state_t pid);

    int get_event_count() const;
    /// @brief Pop the next state from the queue (returns (2,2) if no events are available)
    Vector2 pop_next_state();
    
    std::optional<tap_event_t> get_next_event_internal();
    Vector2 get_next_state();
    int get_next_pid();
    int get_next_time();

    tap_queue_t &get_queue_internal();

    int get_sample_count() const;
    void set_sample_count_internal(int new_samples);

    tap_label_t add_pin(Vector2 initial_state);
    bool has_pin(tap_label_t label) const;
    bool remove_pin(tap_label_t label);

    /**
     * @brief Attach all sensitive pins of a component to the patch bay.
     * 
     * Assumes component.pins is filled.
     */
    void attach_pins_internal(const tap_component_t &component, tap_label_t label);
    void detach_pins_internal(const tap_component_t &component, tap_label_t label);

    Vector2 get_pin_state(tap_label_t label) const;
    TypedDictionary<tap_label_t, Vector2> all_pin_states() const;
    void set_pin_state(tap_label_t label, Vector2 new_state);

    /**
     * @brief Get the internal pin representation.
     * 
     * Do not allow for external modification of pins.
     */
    std::optional<tap_pin_t> get_pin_internal(tap_label_t label) const;
    tap_event_t *get_state_internal(tap_label_t label);

    /**
     * @brief Clear all pins and their states from the patch bay.
     */
    void clear_pins();

    PackedInt64Array get_pin_connections(tap_label_t label) const;
    TypedDictionary<tap_label_t, PackedInt64Array> get_all_pin_connections() const;
    
    TapPatchBay() = default;
};