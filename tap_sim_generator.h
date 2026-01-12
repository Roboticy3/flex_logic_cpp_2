#pragma once

#include "core/object/class_db.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_dictionary.h"
#include "core/templates/hash_map.h"
#include "core/io/resource.h"
#include "servers/audio/effects/audio_stream_generator.h"
#include "scene/resources/audio_stream_wav.h"

#include "tap_sim.h"

/*
Taking a TapSim, and a mapping of wav files to pids, execute the TapSim by
feeding samples from the wav files to the TapSim.

Output is summed over two lists of output pids, one for standard output and
another for player-controlled output.

Assumes wav files match project mix rate. As samples are requested, the total
simulation time is incremented by the number of samples (* tick_rate). The 
generator then plays forward time on the TapSim until time in the total time
has been reached. For each sample requested, output is computed at the 
corresponding time.

Initialized by TapSimGenerator
*/
class TapSimGeneratorPlayback : public AudioStreamGeneratorPlayback {
  GDCLASS(TapSimGeneratorPlayback, AudioStreamGeneratorPlayback)

  friend class TapSimGenerator;

  TapSim *sim;

  //mapping from pin IDs to wav files
  HashMap<tap_label_t, Ref<AudioStreamWAV>> input_wavs;

  //output pin IDs
  PackedInt64Array output_pids;
  PackedInt64Array player_output_pids;

  int tick_rate;

  //current simulation time in ticks
  tap_time_t current_time = 0;

};

class TapSimGenerator : public AudioStreamGenerator {
  GDCLASS(TapSimGenerator, AudioStreamGenerator)

  friend class TapSimGeneratorPlayback;

  using SourceT = AudioStreamWAV; //in case I want to switch this out later.

  NodePath sim_path;

  //mapping from pin IDs to wav files
  HashMap<tap_label_t, Ref<SourceT>> input_wavs;

  //output pin IDs
  PackedInt64Array output_pids;
  PackedInt64Array player_output_pids;

  //simulation tick rate
  int tick_rate = 1024; //number of simulation ticks per audio sample, player should get a warning when this is expended

  protected:
    static void _bind_methods();

  public:
    NodePath get_sim_path() const;
    void set_sim_path(NodePath new_sim_path);

    Dictionary get_input_wavs() const;
    void set_input_wavs(const Dictionary &new_input_wavs);

    PackedInt64Array get_output_pids() const;
    void set_output_pids(const PackedInt64Array &new_output_pids);

    PackedInt64Array get_player_output_pids() const;
    void set_player_output_pids(const PackedInt64Array &new_player_output_pids);

    int get_tick_rate() const;
    void set_tick_rate(int new_tick_rate);

    virtual Ref<AudioStreamPlayback> instantiate_playback() override;
};