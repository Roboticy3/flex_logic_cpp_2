#pragma once

#include "core/object/ref_counted.h"
#include "core/variant/variant.h"
#include "servers/audio/audio_stream.h"

#include "tap_circuit_types.h"
#include "tap_circuit.h"
#include "reference_sim.h"

/**
 * @brief A TapCircuit driver that maps AudioStreams to input pids for input,
 * and sums output from output pids.
 *
 * @param input_streams A vector of AudioStreams mapped to circuit pids.
 * @param debug_input_override If a stream is mapped to this label, pipe
 * directly to the output instead of through the circuit. Good for toggling.
 * @param output_pids Pids that should be summed for the output
 *
 * @param circuit The TapCircuit to simulate.
 * @param reference_sim A function to test the current circuit against
 *
 * @param sample_skip Divides the number of samples passed to the circuit per 
 * second.
 * @param tick_rate Multiplies the number of real samples passed to time passed 
 * in the circuit.
 *
 * @param trackers Internal state for tracking playback progress.
 */
class AudioStreamTapSimulator : public AudioStream {
  GDCLASS(AudioStreamTapSimulator, AudioStream);
  friend class AudioStreamTapSimulatorPlayback;

  struct stream_pid_t {
    Ref<AudioStream> stream;
    tap_label_t pid;
  };

  LocalVector<stream_pid_t> input_streams;
  tap_label_t debug_input_override = -1;
  
  PackedInt64Array output_pids;

  Ref<TapCircuit> circuit;
  Ref<ReferenceSim> reference_sim;

  int sample_skip = 2;
  int tick_rate = 1024;

  struct playback_tracker_t {
    Ref<AudioStreamPlayback> playback;
    size_t event_count;
  };

  HashMap<tap_label_t,playback_tracker_t> trackers;

  bool calculate_stats = true;

protected:
  static void _bind_methods();

public:
  TypedDictionary<tap_label_t, Ref<AudioStream>> get_input_streams() const;
  void set_input_streams(const TypedDictionary<tap_label_t, Ref<AudioStream>> &streams);

  tap_label_t get_debug_input_override() const;
  void set_debug_input_override(tap_label_t label);

  PackedInt64Array get_output_pids() const;
  void set_output_pids(const PackedInt64Array &pids);

  Ref<TapCircuit> get_circuit() const;
  void set_circuit(Ref<TapCircuit> circuit);

  Ref<ReferenceSim> get_reference_sim() const;
  void set_reference_sim(Ref<ReferenceSim> reference_sim);

  int get_sample_skip() const;
  void set_sample_skip(int sample_skip);

  int get_tick_rate() const;
  void set_tick_rate(int tick_rate);

  /**
   * @brief Returns true if all tracked playbacks are playing.
   */
  bool is_simulating() const;

  /**
   * @brief Returns true if the circuit is valid and all input and output pids
   * exist in the circuit.
   */
  bool can_simulate() const;

  /**
   * @brief Returns the number of events pushed to each input pid in total.
   */
  PackedInt64Array get_event_counts() const;

  virtual Ref<AudioStreamPlayback> instantiate_playback() override;
};

/**
 * @brief Drives a TapCircuit by pushing events from `owner->input_streams` to
 * and reads state from `owner->output_pids` to generate audio.
 *
 * @param mix_buffer A temporary buffer for audio.
 *
 * @param owner The AudioStreamTapSimulator that owns this playback.
 * @param current_time The current time in the circuit.
 *
 * @param debug_input_pids Pids that can be piped directly to the output
 * instead of through the circuit. Good for toggling.
 *
 * @param problem `mix_out`'s input pids state before each circuit execution.
 * Used to validate circuit behavior against `owner->reference_sim`.
 * @param solution `mix_out`'s output pids state after each circuit execution.
 * Used to validate circuit behavior against `owner->reference_sim`.
 * @param mix_rate The sample rate of the audio being mixed. Used to compute
 * delta time when incrementing `owner->reference_sim`'s error.
 *
 * @param processed_events_count The number of events processed by the circuit.
 */
class AudioStreamTapSimulatorPlayback : public AudioStreamPlaybackResampled {
  GDCLASS(AudioStreamTapSimulatorPlayback, AudioStreamPlaybackResampled);
  friend class AudioStreamTapSimulator;

  enum {
		MIX_BUFFER_SIZE = 128
	};

  AudioFrame mix_buffer[MIX_BUFFER_SIZE];

  AudioStreamTapSimulator *owner = nullptr;
  tap_time_t current_time = 0;
  size_t processed_events_count = 0;

  HashSet<tap_label_t> debug_input_pids;

  LocalVector<AudioFrame> problem;
  LocalVector<AudioFrame> solution;
  double mix_rate = 44100.0;

protected:
  static void _bind_methods();

public:

  /**
   * @brief Reads circuit state out from pid `owner->debug_input_override`. In
   * normal function, this should sound exactly like the input mapped to that
   * pid, or silence.
   */
  int mix_debug(AudioFrame *p_buffer, float p_rate_scale, int p_frames);

  /**
   * @brief Mix `owner->input_streams` into the circuit at their target pids.
   */
  int mix_in(float p_rate_scale, int p_frames);

  /**
   * @brief Sum the states of `owner->output_pids` into `p_buffer` for each 
   * audio frame.
   */
  int mix_out(AudioFrame *p_buffer, float p_rate_scale, int p_frames);

  /**
   * @brief Run statistics on mixed outputs if `owner->calculate_stats` is true.
   *
   * Prints the average level for each channel to std::cout.
   */
  int mix_stats(AudioFrame *p_buffer, float p_rate_scale, int p_frames);

	/**
   * @brief Schedule and call all the other mix methods.
   */
	virtual int mix(AudioFrame *p_buffer, float p_rate_scale, int p_frames) override;

	/**
   * @brief Start adding events at the latest event position of the simulator
   * and set the live switch on our AudioStreamTapSimulator owner to true.
   * 
   * Note that setting live can fail, in which case an error prints and nothing
   * happens.
   */
	virtual void start(double p_from_pos = 0.0) override;
  /**
   * @brief Sets live to false.
   */
	virtual void stop() override;

  /**
   * @brief Returns whether the stream is currently playing (live switch is on).
   */
	virtual bool is_playing() const override;

	AudioStreamTapSimulatorPlayback() = default;
	~AudioStreamTapSimulatorPlayback() = default;
};